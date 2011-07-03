// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations"

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "Solver/Actions/Proto/Expression.hpp"

#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/Log.hpp"
#include "Common/MPI/PE.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/ElementData.hpp"

#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/SF/Types.hpp"

#include "Solver/CEigenLSS.hpp"
#include "Solver/CTime.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "UFEM/LinearProblem.hpp"
#include "UFEM/UnsteadyModel.hpp"

using namespace CF;
using namespace CF::Solver;
using namespace CF::Solver::Actions;
using namespace CF::Solver::Actions::Proto;
using namespace CF::Common;
using namespace CF::Math::MathConsts;
using namespace CF::Mesh;

using namespace boost;

typedef std::vector<std::string> StringsT;
typedef std::vector<Uint> SizesT;

/// Check close, fo testing purposes
inline void 
check_close(const Real a, const Real b, const Real threshold)
{
  BOOST_CHECK_CLOSE(a, b, threshold);
}

static boost::proto::terminal< void(*)(Real, Real, Real) >::type const _check_close = {&check_close};

struct ProtoUnsteadyFixture
{
  ProtoUnsteadyFixture() :
    length(5.),
    ambient_temp(500.),
    initial_temp(150.),
    nb_segments(100),
    k(1.),
    alpha(1.),
    start_time(0.),
    end_time(10.),
    dt(0.05),
    t(start_time),
    write_interval(100)
  {
  }
  
  /// Write the analytical solution, according to "A Heat transfer textbook", section 5.3
  void set_analytical_solution(CRegion& region, const std::string& field_name, const std::string& var_name)
  {
    MeshTerm<0, ScalarField > T(field_name, var_name);
    
    if(t == 0.)
    {
      for_each_node
      (
        region,
        T = initial_temp
      );
    }
    else
    {
      // Zero the field
      for_each_node
      (
        region,
        T = 0.
      );
      
      const Real Fo = alpha * t / (0.25*length*length); // Fourier number
      for(Uint i = 0; i != 100; ++i) // First 100 (to be sure ;) terms of the series that makes up the analytical solution (in terms of adimensional temperature)
      {
        const Real n = 1. + 2. * static_cast<Real>(i);
        for_each_node
        (
          region,
          T += 4./(pi()*n) * _exp( -0.25*n*n*pi()*pi()*Fo ) * _sin(0.5*n*pi()*(coordinates[0]/(0.5*length)))
        );
      }
      
      // Convert the result from the adimensional form to a real temperature
      for_each_node
      (
        region,
        T = T*(initial_temp - ambient_temp) + ambient_temp
      );
    }
  }
  
  const Real length;
  const Real ambient_temp;
  const Real initial_temp;
  const Uint nb_segments;
  const Real k;
  const Real alpha;
  const Real start_time;
  const Real end_time;
  const Real dt;
  const Uint write_interval;
  Real t;
};

BOOST_FIXTURE_TEST_SUITE( ProtoUnsteadySuite, ProtoUnsteadyFixture )

BOOST_AUTO_TEST_CASE( Heat1DUnsteady )
{
  // Setup a UFEM model
  UFEM::UnsteadyModel& model = Core::instance().root().create_component<UFEM::UnsteadyModel>("Model");
  CMesh& mesh = model.get_child("Mesh").as_type<CMesh>();
  Tools::MeshGeneration::create_line(mesh, length, nb_segments);

  // Linear system setup (TODO: sane default config for this, so this can be skipped)
  CEigenLSS& lss = model.create_component<CEigenLSS>("LSS");
  lss.set_config_file(boost::unit_test::framework::master_test_suite().argv[1]);
  model.problem().solve_action().configure_option("lss", lss.uri());
  
  // Proto placeholders
  MeshTerm<0, ScalarField> temperature("Temperature", "T");
  MeshTerm<1, ScalarField> temperature_analytical("TemperatureAnalytical", "T");
  
  // shorthand for the problem and boundary conditions
  UFEM::LinearProblem& p = model.problem();
  UFEM::BoundaryConditions& bc = p.boundary_conditions();
  
  // Allowed elements (reducing this list improves compile times)
  boost::mpl::vector1<Mesh::SF::Line1DLagrangeP1> allowed_elements;

  // add the top-level actions (assembly, BC and solve)
  model << model.add_action("Initialize", nodes_expression(temperature = initial_temp))
  << model.add_action("InitializeAnalytical", nodes_expression(temperature_analytical = initial_temp)) <<
  (
    p << p.add_action("Assembly", elements_expression
        (
          allowed_elements,
          group <<
          (
            _A = _0, _T = _0,
            element_quadrature <<
            (
              _A(temperature) += alpha * transpose(nabla(temperature))*nabla(temperature),
              _T(temperature) += model.invdt() * transpose(N(temperature))*N(temperature)
            ),
            p.system_matrix += _T + 0.5 * _A,
            p.system_rhs -= _A * nodal_values(temperature)
          )
        ))
      << ( bc << bc.add_constant_bc("xneg", "Temperature", ambient_temp) << bc.add_constant_bc("xpos", "Temperature", ambient_temp) )
      << p.solve_action()
      << p.add_action("Increment", nodes_expression(temperature += p.solution(temperature)))
  );
  
  // Configure timings
  model.time().configure_option("time_step", dt);
  model.time().configure_option("end_time", end_time);
      
  // Run the solver
  model.execute();
  
  // Check result
  t = model.time().time();
  std::cout << "Checking solution at time " << t << std::endl;
  set_analytical_solution(mesh.topology(), "TemperatureAnalytical", "T");
  for_each_node
  (
    mesh.topology(),
    _check_close(temperature_analytical, temperature, 1.)
  );
};

BOOST_AUTO_TEST_SUITE_END()
