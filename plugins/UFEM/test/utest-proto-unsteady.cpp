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

#include "UFEM/LinearSolverUnsteady.hpp"
#include "UFEM/Tags.hpp"
#include "solver/actions/ZeroLSS.hpp"
#include "solver/actions/SolveLSS.hpp"


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

BOOST_AUTO_TEST_CASE( InitMPI )
{
  common::PE::Comm::instance().init(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv);
  BOOST_CHECK_EQUAL(common::PE::Comm::instance().size(), 1);
}

BOOST_AUTO_TEST_CASE( Heat1DUnsteady )
{
  // debug output
  Core::instance().environment().options().configure_option("log_level", 4u);

  // Setup a model
  ModelUnsteady& model = *Core::instance().root().create_component<ModelUnsteady>("Model");
  Domain& domain = model.create_domain("Domain");
  UFEM::LinearSolverUnsteady& solver = *model.create_component<UFEM::LinearSolverUnsteady>("Solver");

  // Linear system setup (TODO: sane default config for this, so this can be skipped)
  math::LSS::System& lss = *model.create_component<math::LSS::System>("LSS");
  lss.options().configure_option("solver", std::string("Trilinos"));
  solver.options().configure_option("lss", lss.handle<math::LSS::System>());
  
  boost::shared_ptr<solver::actions::Iterate> time_loop = allocate_component<solver::actions::Iterate>("TimeLoop");
  time_loop->create_component<solver::actions::CriterionTime>("CriterionTime");

  // Proto placeholders
  MeshTerm<0, ScalarField> temperature("Temperature", UFEM::Tags::solution());
  MeshTerm<1, ScalarField> temperature_analytical("TemperatureAnalytical", UFEM::Tags::source_terms());

  // Allowed elements (reducing this list improves compile times)
  boost::mpl::vector1<mesh::LagrangeP1::Line1D> allowed_elements;

  // BCs
  boost::shared_ptr<UFEM::BoundaryConditions> bc = allocate_component<UFEM::BoundaryConditions>("BoundaryConditions");

  // add the top-level actions (assembly, BC and solve)
  solver
    << create_proto_action("Initialize", nodes_expression(temperature = initial_temp))
    << create_proto_action("InitializeAnalytical", nodes_expression(temperature_analytical = initial_temp))
    <<
    (
      time_loop
      << allocate_component<solver::actions::ZeroLSS>("ZeroLSS")
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
              _T(temperature) += solver.invdt() * transpose(N(temperature))*N(temperature)
            ),
            solver.system_matrix += _T + 0.5 * _A,
            solver.system_rhs += -_A * nodal_values(temperature)
          )
        )
      )
      << bc
      << allocate_component<solver::actions::SolveLSS>("SolveLSS")
      << create_proto_action("Increment", nodes_expression(temperature += solver.solution(temperature)))
      << allocate_component<solver::actions::AdvanceTime>("AdvanceTime")
    );

  // Setup physics
  model.create_physics("cf3.physics.DynamicModel");

  // Setup mesh
  // Mesh& mesh = *domain.create_component<Mesh>("Mesh");
  // Tools::MeshGeneration::create_line(mesh, length, nb_segments);
  boost::shared_ptr<MeshGenerator> create_line = build_component_abstract_type<MeshGenerator>("cf3.mesh.SimpleMeshGenerator","create_line");
  create_line->options().configure_option("mesh",domain.uri()/"Mesh");
  create_line->options().configure_option("lengths",std::vector<Real>(DIM_1D, length));
  create_line->options().configure_option("nb_cells",std::vector<Uint>(DIM_1D, nb_segments));
  Mesh& mesh = create_line->generate();

  lss.matrix()->options().configure_option("settings_file", std::string(boost::unit_test::framework::master_test_suite().argv[1]));

  bc->add_constant_bc("xneg", "Temperature", ambient_temp);
  bc->add_constant_bc("xpos", "Temperature", ambient_temp);

  // Configure timings
  Time& time = model.create_time();
  time.options().configure_option("time_step", dt);
  time.options().configure_option("end_time", end_time);

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
