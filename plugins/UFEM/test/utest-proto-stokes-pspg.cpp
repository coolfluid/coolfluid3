// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations"

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

BOOST_AUTO_TEST_SUITE( ProtoStokesPSPGSuite )

/// Check close, for testing purposes
inline void 
check_close(const Real a, const Real b, const Real threshold)
{
  BOOST_CHECK_SMALL(a - b, threshold);
}

static boost::proto::terminal< void(*)(Real, Real, Real) >::type const _check_close = {&check_close};

// Solve the Stokes equations with PSPG
BOOST_AUTO_TEST_CASE( ProtoStokesPSPG )
{
  const Real length = 5.;
  const Real height = 2.;
  const Uint x_segments = 25;
  const Uint y_segments = 10;
  
  const Real start_time = 0.;
  const Real end_time = 10.;
  const Real dt = 0.5;
  Real t = start_time;
  const Uint write_interval = 200;
  const Real invdt = 1. / dt;
  
  const Real mu = 0.1;
  const Real rho = 100.;
  const Real nu = mu / rho;
  
  const RealVector2 u_direction(1., 0.);
  RealVector u_wall(2); u_wall.setZero();
  const Real p0 = 5.;
  const Real p1 = 0.;
  const Real c = 0.5 * (p0 - p1) / (rho * mu * length);
  
  // Setup a UFEM model
  UFEM::UnsteadyModel& model = Core::instance().root().create_component<UFEM::UnsteadyModel>("Model");
  CMesh& mesh = model.get_child("Mesh").as_type<CMesh>();
  Tools::MeshGeneration::create_rectangle(mesh, length, height, x_segments, y_segments);

  // Linear system setup (TODO: sane default config for this, so this can be skipped)
  CEigenLSS& lss = model.create_component<CEigenLSS>("LSS");
  lss.set_config_file(boost::unit_test::framework::master_test_suite().argv[1]);
  model.problem().solve_action().configure_option("lss", lss.uri());

  // shorthand for the problem and boundary conditions
  UFEM::LinearProblem& pr = model.problem();
  UFEM::BoundaryConditions& bc = pr.boundary_conditions();
  
  // Allowed elements (reducing this list improves compile times)
  boost::mpl::vector1<Mesh::SF::Quad2DLagrangeP1> allowed_elements;
  
  // Expression variables
  MeshTerm<0, VectorField> u("Velocity", "u");
  MeshTerm<1, ScalarField> p("Pressure", "p");
  
  SUPGCoeffs coefs;
  coefs.u_ref = c;
  coefs.nu = mu / rho;
  coefs.rho = rho;
  
  // Special boundary conditions
  CAction& left_bc = bc.add_action("LeftBC", nodes_expression(pr.dirichlet(u) = c * coordinates[1] * (height - coordinates[1]) * u_direction));
  CAction& right_bc = bc.add_action("RightBC", nodes_expression(pr.dirichlet(u) = c * coordinates[1] * (height - coordinates[1]) * u_direction));
  left_bc.configure_option("region", find_component_ptr_recursively_with_name<CRegion>(mesh.topology(), "left"));
  right_bc.configure_option("region", find_component_ptr_recursively_with_name<CRegion>(mesh.topology(), "right"));
  
  // build up the solver out of different actions
  model
  << model.add_action("InitializePressure", nodes_expression(p = 0.))
  << model.add_action("InitializeVelocity", nodes_expression(u = c * coordinates[1] * (height - coordinates[1]) * u_direction)) <<
  ( // Time loop
    pr << pr.add_action
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
            _A(p    , u[_i]) +=          transpose(N(p))         * nabla(u)[_i], // Continuity, standard
            _A(p    , p)     += coefs.tau_ps * transpose(nabla(p))     * nabla(p),     // Continuity, PSPG
            _A(u[_i], u[_i]) += mu     * transpose(nabla(u))     * nabla(u),     // Diffusion
            _A(u[_i], p)     += 1./rho * transpose(N(u))         * nabla(p)[_i], // Pressure gradient
            _T(p    , u[_i]) += coefs.tau_ps * transpose(nabla(p)[_i]) * N(u),         // Time, PSPG
            _T(u[_i], u[_i]) += transpose(N(u))         * N(u)          // Time, standard
          ),
          pr.system_matrix += invdt * _T + 0.5 * _A,
          pr.system_rhs -= _A * _b
        )
      )
    ) <<
    ( // boundary conditions
      bc
      << bc.add_constant_bc("left", "Pressure", p0)
      << bc.add_constant_bc("right", "Pressure", p1)
      << left_bc << right_bc
      << bc.add_constant_bc("bottom", "Velocity", u_wall)
      << bc.add_constant_bc("top", "Velocity", u_wall)
    )
    << pr.solve_action() // solve
    << pr.add_action("IncrementU", nodes_expression(u += pr.solution(u))) // increment solution
    << pr.add_action("IncrementP", nodes_expression(p += pr.solution(p)))
  )
  << model.add_action("CheckP", nodes_expression(_check_close(p, p0 * (length - coordinates[0]) / length + p1 * coordinates[1] / length, 1e-3)))
  << model.add_action("CheckU", nodes_expression(_check_close(u[0], c * coordinates[1] * (height - coordinates[1]), 1e-2)))
  << model.add_action("CheckV", nodes_expression(_check_close(u[1], 0., 1e-3)));

  // Configure timings
  model.time().configure_option("time_step", dt);
  model.time().configure_option("end_time", end_time);
      
  // Run the solver
  model.execute();
  
  // Write result
  URI output_file("stokes-pspg.msh");
  model.configure_option("output_file", output_file);
  SignalArgs a;
  model.signal_write_mesh(a);
}

BOOST_AUTO_TEST_SUITE_END()
