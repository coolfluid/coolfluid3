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

#include "solver/CModelUnsteady.hpp"
#include "solver/CTime.hpp"
#include "solver/Tags.hpp"

#include "solver/actions/Proto/CProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "UFEM/LinearSolverUnsteady.hpp"
#include "UFEM/NavierStokesOps.hpp"
#include "UFEM/Tags.hpp"
#include "UFEM/TimeLoop.hpp"

#include "NavierStokes.hpp"

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
  Core::instance().environment().configure_option("log_level", 4u);

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

  const Real mu = 0.01;
  const Real rho = 100.;
  const Real epsilon = rho/mu;

  RealVector u_wall(2);
  u_wall.setZero();
  const Real p0 = 5.;
  const Real p1 = 0.;
  const Real c = 0.5*(p0 - p1) / (rho * mu * length);
  const RealVector2 u_max(c, 0.);

  SUPGCoeffs coefs;
  coefs.u_ref = c;
  coefs.mu = mu;
  coefs.rho = rho;

  // List of (Navier-)Stokes creation functions, with their names
  const std::vector<std::string> names = boost::assign::list_of("stokes_artifdiss")("stokes_pspg")("navier_stokes_pspg")("navier_stokes_supg")("navier_stokes_bulk");
  typedef Expression::Ptr (*FactoryT)(LinearSolverUnsteady&, SUPGCoeffs&);
  std::vector<FactoryT> factories = boost::assign::list_of(&stokes_artifdiss)(&stokes_pspg)(&navier_stokes_pspg)(&navier_stokes_supg)(&navier_stokes_bulk);

  // Loop over all model types
  for(Uint i = 0; i != names.size(); ++i)
  {
    std::cout << "\n################################## Running test for model " << names[i] << "##################################\n" << std::endl;
    // Setup a model
    CModelUnsteady& model = Core::instance().root().create_component<CModelUnsteady>(names[i]);
    Domain& domain = model.create_domain("Domain");
    LinearSolverUnsteady& solver = model.create_component<LinearSolverUnsteady>("Solver");

    // Linear system setup (TODO: sane default config for this, so this can be skipped)
    math::LSS::System& lss = model.create_component<math::LSS::System>("LSS");
    lss.configure_option("solver", std::string("Trilinos"));
    solver.configure_option("lss", lss.uri());

    // Expression variables
    MeshTerm<0, VectorField> u("Velocity", UFEM::Tags::solution());
    MeshTerm<1, ScalarField> p("Pressure", UFEM::Tags::solution());

    // build up the solver out of different actions
    solver
      << create_proto_action("InitializePressure", nodes_expression(p = 0.))
      << create_proto_action("InitializeVelocity", parabolic_field(solver, u_max, height))
      <<
      ( // Time loop
        solver.create_component<TimeLoop>("TimeLoop")
        << solver.zero_action()
        << create_proto_action("Assembly", factories[i](solver, coefs))
        << solver.boundary_conditions()
        << solver.solve_action()
        << create_proto_action("IncrementU", nodes_expression(u += solver.solution(u)))
        << create_proto_action("IncrementP", nodes_expression(p += solver.solution(p)))
      )
      << create_proto_action("CheckP", nodes_expression(_check_close(p, p0 * (length - coordinates[0]) / length + p1 * coordinates[1] / length, 6e-3)))
      << create_proto_action("CheckU", nodes_expression(_check_close(u[0], c * coordinates[1] * (height - coordinates[1]), 1e-2)))
      << create_proto_action("CheckV", nodes_expression(_check_close(u[1], 0., 6e-3)));

    // Setup physics
    model.create_physics("cf3.physics.DynamicModel");

    // Setup mesh
    Mesh& mesh = domain.create_component<Mesh>("Mesh");
    Tools::MeshGeneration::create_rectangle(mesh, length, height, x_segments, y_segments);

    lss.matrix()->configure_option("settings_file", std::string(boost::unit_test::framework::master_test_suite().argv[1]));

    solver.boundary_conditions().add_constant_bc("left", "Pressure", p0);
    solver.boundary_conditions().add_constant_bc("right", "Pressure", p1);
    solver.boundary_conditions().add_constant_bc("bottom", "Velocity", u_wall);
    solver.boundary_conditions().add_constant_bc("top", "Velocity", u_wall);
    solver.boundary_conditions() << create_proto_action("ParabolicBC", parabolic_dirichlet(solver, u_max, height));

    // Set the region of the parabolic inlet and outlet
    const std::vector<URI> in_out = boost::assign::list_of(find_component_recursively_with_name<Region>(mesh.topology(), "left").uri())
                                                          (find_component_recursively_with_name<Region>(mesh.topology(), "right").uri());

    solver.boundary_conditions().get_child("ParabolicBC").configure_option(solver::Tags::regions(), in_out);

    // Configure timings
    CTime& time = model.create_time();
    time.configure_option("time_step", dt);
    time.configure_option("end_time", end_time);

    // Run the solver
    model.simulate();
  }
}

BOOST_AUTO_TEST_SUITE_END()
