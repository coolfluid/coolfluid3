// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations"

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "Solver/Actions/Proto/ElementLooper.hpp"
#include "Solver/Actions/Proto/Functions.hpp"
#include "Solver/Actions/Proto/NodeLooper.hpp"
#include "Solver/Actions/Proto/Terminals.hpp"

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

#include "Tools/MeshGeneration/MeshGeneration.hpp"

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
    end_time(10),
    dt(0.01),
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
  const Real invdt = 1. / dt;

  // Setup document structure and mesh
  CRoot& root = Core::instance().root();
  
  CMesh::Ptr mesh = root.create_component_ptr<CMesh>("mesh");
  Tools::MeshGeneration::create_line(*mesh, length, nb_segments);
  
  // Linear system
  CEigenLSS& lss = *root.create_component_ptr<CEigenLSS>("LSS");
  lss.set_config_file(boost::unit_test::framework::master_test_suite().argv[1]);
  
  // Create output field
  lss.resize(mesh->create_scalar_field("Temperature", "T", CField::Basis::POINT_BASED).data().size());
  
  // Create a field for the analytical solution
  mesh->create_scalar_field("TemperatureAnalytical", "T", CField::Basis::POINT_BASED);
  
  // Regions
  CRegion& xneg = find_component_recursively_with_name<CRegion>(*mesh, "xneg");
  CRegion& xpos = find_component_recursively_with_name<CRegion>(*mesh, "xpos");

  MeshTerm<0, ScalarField> temperature("Temperature", "T");
  MeshTerm<1, ScalarField> temperature_analytical("TemperatureAnalytical", "T");
  
  
  // Set initial condition.
  set_analytical_solution(mesh->topology(), "Temperature", "T");

// Analytical derivation of the element matrices:
//  const Real seg_length = length / static_cast<Real>(nb_segments);
//   RealMatrix2 A;
//   A << 1, -1, -1, 1;
//   A *= alpha / seg_length;
//   
//   RealMatrix2 T;
//   T << 1. / 3., 1. / 6. , 1. / 6., 1. / 3.;
//   T *= invdt * seg_length;
  
  while(t < end_time)
  { 
    // Fill the system matrix
    lss.set_zero();
    for_each_element< boost::mpl::vector1<SF::Line1DLagrangeP1> >
    (
      mesh->topology(),
      group <<
      (
        _A(temperature) = alpha * integral<1>(laplacian_elm(temperature) * jacobian_determinant),
        _T(temperature) = invdt * integral<1>(value_elm(temperature) * jacobian_determinant),
        system_matrix(lss) += _T + 0.5 * _A,
        system_rhs(lss) -= _A * temperature
      )
    );
    
    // Left boundary at ambient temperature
    for_each_node
    (
      xneg,
      dirichlet(lss, temperature) = ambient_temp
    );
    
    // Right boundary at ambient temperature
    for_each_node
    (
      xpos,
      dirichlet(lss, temperature) = ambient_temp
    );
    
    // Solve the system!
    lss.solve();
    increment_solution(lss.solution(), StringsT(1, "Temperature"), StringsT(1, "T"), SizesT(1, 1), *mesh);
    
    t += dt;
        
    // Output solution (pylab-compatible)
    if(t > 0. && (static_cast<Uint>(t / dt) % write_interval == 0 || t >= end_time))
    {
      std::cout << "PyVarX = [";
      for_each_node
      (
        mesh->topology(),
        _cout << coordinates << ", "
      );
      std::cout << "]\n";
      
      std::cout << "PyVarT = [";
      for_each_node
      (
        mesh->topology(),
        _cout << temperature << ", "
      );
      std::cout << "]\n";
      
      std::cout << "PyVarLabel = \'t = " << t << "\'\n";
      
      set_analytical_solution(mesh->topology(), "TemperatureAnalytical", "T");
      
      std::cout << "Checking solution at t = " << t << std::endl;
      
      for_each_node
      (
        mesh->topology(),
        _check_close(temperature_analytical, temperature, 1.)
      );
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
