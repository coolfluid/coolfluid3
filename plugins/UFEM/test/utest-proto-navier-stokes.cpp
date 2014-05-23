// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations"

#include <boost/test/unit_test.hpp>

#include "common/Core.hpp"
#include "common/Environment.hpp"

#include "mesh/Domain.hpp"

#include "solver/ModelUnsteady.hpp"
#include "solver/Time.hpp"
#include "solver/Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Iterate.hpp"
#include "solver/CriterionTime.hpp"
#include "solver/actions/AdvanceTime.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"
#include "mesh/MeshGenerator.hpp"

#include "UFEM/LSSActionUnsteady.hpp"
#include "UFEM/Solver.hpp"
#include "UFEM/Tags.hpp"
#include "UFEM/ParsedFunctionExpression.hpp"

#include "NavierStokes.hpp"
#include "math/LSS/ZeroLSS.hpp"
#include "math/LSS/SolveLSS.hpp"

using namespace cf3;
using namespace cf3::solver;
using namespace cf3::solver::actions;
using namespace cf3::solver::actions::Proto;
using namespace cf3::common;
using namespace cf3::math::Consts;
using namespace cf3::mesh;
using namespace cf3::UFEM;

using namespace boost;

typedef std::vector<std::string> StringsT;
typedef std::vector<Uint> SizesT;

BOOST_AUTO_TEST_SUITE( ProtoStokesArtifDissSuite )

inline void
check_close(const Real a, const Real b, const Real threshold)
{
  BOOST_CHECK_SMALL(a - b, threshold);
}

static boost::proto::terminal< void(*)(Real, Real, Real) >::type const _check_close = {&check_close};

BOOST_AUTO_TEST_CASE( InitMPI )
{
  common::PE::Comm::instance().init(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv);
  BOOST_CHECK_EQUAL(common::PE::Comm::instance().size(), 1);
}

// Solve the Stokes equations with artificial dissipation
BOOST_AUTO_TEST_CASE( ProtoNavierStokes )
{
  // debug output
  Core::instance().environment().options().set("log_level", 1u);

  const Real length = 5.;
  const Real height = 2.;
  const Uint x_segments = 25;
  const Uint y_segments = 10;

  const Real start_time = 0.;
  const Real end_time = 5.;
  const Real dt = 1.;
  Real t = start_time;
  const Uint write_interval = 5000;
  const Real invdt = 1. / dt;

  const Real mu = 1.;
  const Real rho = 1.;

  const std::vector<Real> u_wall(2, 0.);
  const Real p0 = 10.;
  const Real p1 = 0.;
  const Real c = 0.5*(p0 - p1) / (rho * mu * length);
  const RealVector2 u_max(c, 0.);

  // Parabolic expression for the inlet
  std::stringstream parabole_str;
  parabole_str << "y*(" << height << "-y)*" << c;
  std::vector<std::string> parabole_functions(2, "0");
  parabole_functions[0] = parabole_str.str();


  // List of (Navier-)Stokes creation functions, with their names
  const std::vector<std::string> names = boost::assign::list_of("stokes_artifdiss")("stokes_pspg")("navier_stokes_pspg")("navier_stokes_supg");
  typedef boost::shared_ptr< ProtoAction > (*FactoryT)(LSSActionUnsteady&);
  std::vector<FactoryT> factories = boost::assign::list_of(&stokes_artifdiss)(&stokes_pspg)(&navier_stokes_pspg)(&navier_stokes_supg);

  // Loop over all model types
  for(Uint i = 0; i != names.size(); ++i)
  {
    std::cout << "\n################################## Running test for model " << names[i] << "##################################\n" << std::endl;
    // Setup a model
    ModelUnsteady& model = *Core::instance().root().create_component<ModelUnsteady>(names[i]);
    Domain& domain = model.create_domain("Domain");
    UFEM::Solver& solver = *model.create_component<UFEM::Solver>("Solver");
    Handle<UFEM::LSSActionUnsteady> lss_action(solver.add_unsteady_solver("cf3.UFEM.LSSActionUnsteady"));
    lss_action->set_solution_tag("navier_stokes_solution");
    Handle<common::ActionDirector> ic(solver.get_child("InitialConditions"));

    boost::shared_ptr<solver::actions::Iterate> time_loop = allocate_component<solver::actions::Iterate>("TimeLoop");
    time_loop->create_component<solver::CriterionTime>("CriterionTime");

    // Expression variables
    FieldVariable<0, VectorField> u("Velocity", "navier_stokes_solution");
    FieldVariable<1, ScalarField> p("Pressure", "navier_stokes_solution");
    FieldVariable<2, VectorField> u_adv("AdvectionVelocity", "linearized_velocity");
    FieldVariable<3, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");

    PhysicsConstant nu("kinematic_viscosity");

    // Velocity initial condition
    boost::shared_ptr<UFEM::ParsedFunctionExpression> vel_init = allocate_component<UFEM::ParsedFunctionExpression>("InitializeVelocity");
    vel_init->set_expression(nodes_expression(u = vel_init->vector_function()));
    vel_init->options().set("value", parabole_functions);

    *ic << create_proto_action("InitializePressure", nodes_expression(p = 0.)) << vel_init;

    // BC
    boost::shared_ptr<UFEM::BoundaryConditions> bc = allocate_component<UFEM::BoundaryConditions>("BoundaryConditions");
    bc->set_solution_tag("navier_stokes_solution");

    // build up the solver out of different actions
    *lss_action
        << create_proto_action("AdvectionVel", nodes_expression(u_adv = u))
        << create_proto_action("InitNu", nodes_expression(nu_eff = nu))
        << allocate_component<math::LSS::ZeroLSS>("ZeroLSS")
        << factories[i](*lss_action)
        << bc
        << allocate_component<math::LSS::SolveLSS>("SolveLSS")
        << create_proto_action("IncrementU", nodes_expression(u += lss_action->solution(u)))
        << create_proto_action("IncrementP", nodes_expression(p += lss_action->solution(p)));
    solver
      << create_proto_action("CheckP", nodes_expression(_check_close(p, p0 * (length - coordinates[0]) / length + p1 * coordinates[1] / length, 1.)))
      << create_proto_action("CheckU", nodes_expression(_check_close(u[0], c * coordinates[1] * (height - coordinates[1]), 5e-2)))
      << create_proto_action("CheckV", nodes_expression(_check_close(u[1], 0., 6e-3)));

    Handle<Component> ns_solver = Core::instance().root().get_child("ns_solver");

    // Setup physics
    physics::PhysModel& physical_model = model.create_physics("cf3.UFEM.NavierStokesPhysics");
    if(is_not_null(ns_solver))
    {
      ns_solver->options().set("physical_model", physical_model.handle<physics::PhysModel>());
    }
    physical_model.options().set("density", rho);
    physical_model.options().set("dynamic_viscosity", mu);

    // Setup mesh
    boost::shared_ptr<MeshGenerator> create_rectangle = build_component_abstract_type<MeshGenerator>("cf3.mesh.SimpleMeshGenerator","create_line");
    create_rectangle->options().set("mesh",domain.uri()/"Mesh");
    std::vector<Real> lengths(2);     lengths[XX] = length;            lengths[YY]  = height;
    std::vector<Uint> nb_cells(2);    nb_cells[XX] = x_segments;       nb_cells[YY] = y_segments;
    create_rectangle->options().set("lengths",lengths);
    create_rectangle->options().set("nb_cells",nb_cells);
    Mesh& mesh = create_rectangle->generate();

    solver.configure_option_recursively("regions", std::vector<URI>(1, mesh.topology().uri()));

    if(is_not_null(ns_solver))
    {
      ns_solver->options().set("lss", lss_action->get_child("LSS"));
    }

    // Boundary conditions
    bc->add_constant_bc("left", "Pressure", p0);
    bc->add_constant_bc("right", "Pressure", p1);
    bc->add_constant_bc("bottom", "Velocity", u_wall);
    bc->add_constant_bc("top", "Velocity", u_wall);
    bc->add_function_bc("left", "Velocity")->options().set("value", parabole_functions);
    //bc->add_function_bc("right", "Velocity")->options().set("value", parabole_functions);

    // Configure timings
    Time& time = model.create_time();
    time.options().set("time_step", dt);
    time.options().set("end_time", end_time);

    // Run the solver
    model.simulate();

    domain.write_mesh("ns-test-" + names[i] + ".pvtu");
  }
}

BOOST_AUTO_TEST_SUITE_END()
