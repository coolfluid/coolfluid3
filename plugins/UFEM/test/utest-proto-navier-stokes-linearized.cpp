// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for linearized laminar Navier-Stokes"

#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>

#include "Solver/Actions/Proto/Expression.hpp"
#include "Solver/CTime.hpp"

#include "Common/Core.hpp" 
#include "Common/CRoot.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "UFEM/LinearProblem.hpp"
#include "UFEM/NavierStokesOps.hpp"
#include "UFEM/UnsteadyModel.hpp"


using namespace CF;
using namespace CF::Solver;
using namespace CF::Solver::Actions;
using namespace CF::Solver::Actions::Proto;
using namespace CF::Common;
using namespace CF::Math::MathConsts;
using namespace CF::Mesh;
using namespace CF::UFEM;

using namespace boost;

typedef std::vector<std::string> StringsT;
typedef std::vector<Uint> SizesT;

/// Probe based on a coordinate value
void probe(const Real coord, const Real val, Real& result)
{
  if(coord > -0.1 && coord < 0.1)
    result = val;
}

static boost::proto::terminal< void(*)(Real, Real, Real&) >::type const _probe = {&probe};

BOOST_AUTO_TEST_SUITE( ProtoNSLinSuite )

// Solve the Navier-Stokes equations with SUPG and the bulk viscosity term
BOOST_AUTO_TEST_CASE( ProtoNavierStokesBULK )
{
  int    argc = boost::unit_test::framework::master_test_suite().argc;
  char** argv = boost::unit_test::framework::master_test_suite().argv;
  
  const Real start_time = 0.;
  const Real end_time = boost::lexical_cast<Real>(argv[1]);
  const Real dt = boost::lexical_cast<Real>(argv[2]);
  Real t = start_time;
  const Uint write_interval = 5;
  const Real invdt = 1. / dt;
  
  const Real mu = 0.01;
  const Real rho = 1.;
  
  RealVector u_inf(2); u_inf << 16, 0.;
  RealVector u_wall(2); u_wall << 0., 0.;
  const Real p_out = 0.;
  
  // Storage for the calculated coefficients
  Real tau_ps, tau_su, tau_bulk;
  
  SUPGCoeffs coefs;
  coefs.u_ref = u_inf[XX];
  coefs.nu = mu / rho;
  coefs.rho = rho;
  
  // Setup a UFEM model
  UFEM::UnsteadyModel& model = Core::instance().root().create_component<UFEM::UnsteadyModel>("Model");

  // Linear system setup (TODO: sane default config for this, so this can be skipped)
  CEigenLSS& lss = model.create_component<CEigenLSS>("LSS");
  lss.set_config_file(boost::unit_test::framework::master_test_suite().argv[3]);
  model.problem().solve_action().configure_option("lss", lss.uri());

  // shorthand for the problem and boundary conditions
  UFEM::LinearProblem& pr = model.problem();
  UFEM::BoundaryConditions& bc = pr.boundary_conditions();
  
  // Allowed elements (reducing this list improves compile times)
  boost::mpl::vector1<Mesh::SF::Triag2DLagrangeP1> allowed_elements;
  
  // Expression variables
  MeshTerm<0, VectorField> u("Velocity", "u");
  MeshTerm<1, ScalarField> p("Pressure", "p");
  MeshTerm<2, VectorField> u_adv("AdvectionVelocity", "u_adv"); // The extrapolated advection velocity (n+1/2)
  MeshTerm<3, VectorField> u1("AdvectionVelocity", "u1");  // Two timesteps ago (n-1)
  MeshTerm<4, VectorField> u2("AdvectionVelocity", "u2"); // n-2
  MeshTerm<5, VectorField> u3("AdvectionVelocity", "u3"); // n-3
  
  URI input_file = URI(argv[4]);
  model.configure_option("input_file", input_file);
  SignalArgs a;
  model.signal_read_mesh(a);
  
  // build up the solver out of different actions
  model
  << model.add_action("InitializePressure", nodes_expression(p = 0.))
  << model.add_action("InitializeVelocity", nodes_expression(u = u_wall))
  << model.add_action("InitializeU1", nodes_expression(u1 = u))
  << model.add_action("InitializeU2", nodes_expression(u2 = u))
  << model.add_action("InitializeU3", nodes_expression(u3 = u)) <<
  ( // Time loop
    pr << pr.add_action("ExtrapolateU", nodes_expression(u_adv = 2.1875*u - 2.1875*u1 + 1.3125*u2 - 0.3125*u3))
    << pr.add_action
    (
      "Assembly",
      elements_expression // assembly
      (
        allowed_elements,
        group <<
        (
          _A = _0, _T = _0,
          compute_tau(u, coefs),
          element_quadrature <<
          (
            _A(p    , u[_i]) +=          transpose(N(p))         * nabla(u)[_i] + coefs.tau_ps * transpose(nabla(p)[_i]) * u_adv*nabla(u), // Standard continuity + PSPG for advection
            _A(p    , p)     += coefs.tau_ps * transpose(nabla(p))     * nabla(p),     // Continuity, PSPG
            _A(u[_i], u[_i]) += mu     * transpose(nabla(u))     * nabla(u)     + transpose(N(u) + coefs.tau_su*u_adv*nabla(u)) * u_adv*nabla(u),     // Diffusion + advection
            _A(u[_i], p)     += 1./rho * transpose(N(u) + coefs.tau_su*u_adv*nabla(u)) * nabla(p)[_i], // Pressure gradient (standard and SUPG)
            _A(u[_i], u[_j]) += coefs.tau_bulk * transpose(nabla(u)[_i]) * nabla(u)[_j], // Bulk viscosity
            _T(p    , u[_i]) += coefs.tau_ps * transpose(nabla(p)[_i]) * N(u),         // Time, PSPG
            _T(u[_i], u[_i]) += invdt  * transpose(N(u) + coefs.tau_su*u_adv*nabla(u))         * N(u)          // Time, standard
          ),
          pr.system_matrix += invdt * _T + 1.0 * _A,
          pr.system_rhs -= _A * _b
        )
      )
    ) <<
    ( // boundary conditions
      bc
      << bc.add_constant_bc("in", "Velocity", u_inf)
      << bc.add_constant_bc("out", "Pressure", p_out)
      << bc.add_constant_bc("symm", "Velocity", u_inf)
      << bc.add_constant_bc("wall", "Velocity", u_wall)
    )
    << pr.solve_action() // solve
    << pr.add_action("SaveU2", nodes_expression(u3 = u2))
    << pr.add_action("SaveU1", nodes_expression(u2 = u1))
    << pr.add_action("SaveU0", nodes_expression(u1 = u))
    << pr.add_action("IncrementU", nodes_expression(u += pr.solution(u))) // increment solution
    << pr.add_action("IncrementP", nodes_expression(p += pr.solution(p)))
  );

  // Configure timings
  model.time().configure_option("time_step", dt);
  model.time().configure_option("end_time", end_time);
      
  // Run the solver
  model.execute();
  
  // Write result
  URI output_file("navier-stokes-linearized.msh");
  model.configure_option("output_file", output_file);
  model.signal_write_mesh(a);
}

BOOST_AUTO_TEST_SUITE_END()
