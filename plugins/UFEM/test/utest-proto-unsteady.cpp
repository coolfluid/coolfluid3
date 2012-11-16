// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations"

#include <boost/test/unit_test.hpp>

#define BOOST_PROTO_MAX_ARITY 10
#ifdef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
 #undef BOOST_MPL_LIMIT_METAFUNCTION_ARITY
 #define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 10
#endif

#include "common/Core.hpp"
#include "common/Environment.hpp"

#include "mesh/Domain.hpp"
#include "mesh/MeshGenerator.hpp"

#include "solver/ModelUnsteady.hpp"
#include "solver/Time.hpp"
#include "solver/actions/Iterate.hpp"
#include "solver/actions/CriterionTime.hpp"
#include "solver/actions/AdvanceTime.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "UFEM/LSSActionUnsteady.hpp"
#include "UFEM/Solver.hpp"
#include "UFEM/Tags.hpp"
#include "math/LSS/ZeroLSS.hpp"
#include "math/LSS/SolveLSS.hpp"


using namespace cf3;
using namespace cf3::solver;
using namespace cf3::solver::actions;
using namespace cf3::solver::actions::Proto;
using namespace cf3::common;
using namespace cf3::math::Consts;
using namespace cf3::mesh;

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
  void set_analytical_solution(Region& region, const std::string& field_name, const std::string& var_name)
  {
    FieldVariable<0, ScalarField > T(field_name, var_name);

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

BOOST_AUTO_TEST_CASE( InitMPI )
{
  common::PE::Comm::instance().init(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv);
  BOOST_CHECK_EQUAL(common::PE::Comm::instance().size(), 1);
}

BOOST_AUTO_TEST_CASE( Heat1DUnsteady )
{
  // debug output
  Core::instance().environment().options().set("log_level", 4u);

  // Setup a model
  ModelUnsteady& model = *Core::instance().root().create_component<ModelUnsteady>("Model");
  Domain& domain = model.create_domain("Domain");
  UFEM::Solver& solver = *model.create_component<UFEM::Solver>("Solver");
  Handle<UFEM::LSSActionUnsteady> lss_action(solver.add_unsteady_solver("cf3.UFEM.LSSActionUnsteady"));
  Handle<common::ActionDirector> ic(solver.get_child("InitialConditions"));


  boost::shared_ptr<solver::actions::Iterate> time_loop = allocate_component<solver::actions::Iterate>("TimeLoop");
  time_loop->create_component<solver::actions::CriterionTime>("CriterionTime");

  // Proto placeholders
  FieldVariable<0, ScalarField> temperature("Temperature", UFEM::Tags::solution());
  FieldVariable<1, ScalarField> temperature_analytical("TemperatureAnalytical", UFEM::Tags::source_terms());

  // Allowed elements (reducing this list improves compile times)
  boost::mpl::vector1<mesh::LagrangeP1::Line1D> allowed_elements;

  // BCs
  boost::shared_ptr<UFEM::BoundaryConditions> bc = allocate_component<UFEM::BoundaryConditions>("BoundaryConditions");

  // add the top-level actions (assembly, BC and solve)
  *ic
    << create_proto_action("Initialize", nodes_expression(temperature = initial_temp))
    << create_proto_action("InitializeAnalytical", nodes_expression(temperature_analytical = initial_temp));
  *lss_action
      << allocate_component<math::LSS::ZeroLSS>("ZeroLSS")
      << create_proto_action
      (
        "Assembly",
        elements_expression
        (
          allowed_elements,
          group
          (
            _A = _0, _T = _0,
            element_quadrature
            (
              _A(temperature) += alpha * transpose(nabla(temperature))*nabla(temperature),
              _T(temperature) += lss_action->invdt() * transpose(N(temperature))*N(temperature)
            ),
            lss_action->system_matrix += _T + 0.5 * _A,
            lss_action->system_rhs += -_A * nodal_values(temperature)
          )
        )
      )
      << bc
      << allocate_component<math::LSS::SolveLSS>("SolveLSS")
      << create_proto_action("Increment", nodes_expression(temperature += lss_action->solution(temperature)));

  // Setup physics
  model.create_physics("cf3.physics.DynamicModel");

  // Setup mesh
  // Mesh& mesh = *domain.create_component<Mesh>("Mesh");
  // Tools::MeshGeneration::create_line(mesh, length, nb_segments);
  boost::shared_ptr<MeshGenerator> create_line = build_component_abstract_type<MeshGenerator>("cf3.mesh.SimpleMeshGenerator","create_line");
  create_line->options().set("mesh",domain.uri()/"Mesh");
  create_line->options().set("lengths",std::vector<Real>(DIM_1D, length));
  create_line->options().set("nb_cells",std::vector<Uint>(DIM_1D, nb_segments));
  Mesh& mesh = create_line->generate();

  lss_action->options().set("regions", std::vector<URI>(1, mesh.topology().uri()));
  ic->get_child("Initialize")->options().set("regions", std::vector<URI>(1, mesh.topology().uri()));
  ic->get_child("InitializeAnalytical")->options().set("regions", std::vector<URI>(1, mesh.topology().uri()));

  bc->add_constant_bc("xneg", "Temperature", ambient_temp);
  bc->add_constant_bc("xpos", "Temperature", ambient_temp);

  // Configure timings
  Time& time = model.create_time();
  time.options().set("time_step", dt);
  time.options().set("end_time", end_time);

  // Run the solver
  model.simulate();

  // Check result
  t = model.time().current_time();
  std::cout << "Checking solution at time " << t << std::endl;
  set_analytical_solution(mesh.topology(), "TemperatureAnalytical", UFEM::Tags::source_terms());
  for_each_node
  (
    mesh.topology(),
    _check_close(temperature_analytical, temperature, 1.)
  );

  std::cout << solver.tree() << std::endl;
};

BOOST_AUTO_TEST_SUITE_END()
