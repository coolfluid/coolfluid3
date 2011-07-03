// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for heat-conduction related proto operations"

#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>

#include "Solver/Actions/Proto/Expression.hpp"
#include "Solver/CTime.hpp"

#include "Common/Core.hpp" 
#include "Common/CRoot.hpp"

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

struct ComputeTau
{ 
  /// Dummy result
  typedef void result_type;
  
  template<typename UT>
  void operator()(const UT& u, const Real u_ref, const Real nu, Real& tau_ps) const
  {
    const Real he=sqrt(4./3.141592654*u.support().volume());
    const Real ree=u_ref*he/(2.*nu);
    const Real xi=std::max(0.,std::min(ree/3.,1.));
    tau_ps = he*xi/(2.*u_ref);
  }
};

/// Placeholder for the compute_tau operation
static MakeSFOp<ComputeTau>::type const compute_tau = {};

/// Probe based on a coordinate value
void probe(const Real coord, const Real val, Real& result)
{
  if(coord > -0.1 && coord < 0.1)
    result = val;
}

static boost::proto::terminal< void(*)(Real, Real, Real&) >::type const _probe = {&probe};

BOOST_AUTO_TEST_SUITE( ProtoSystemSuite )

// Solve the Navier-Stokes equations with PSPG
BOOST_AUTO_TEST_CASE( ProtoNavierStokesPSPG )
{
  int    argc = boost::unit_test::framework::master_test_suite().argc;
  char** argv = boost::unit_test::framework::master_test_suite().argv;
  
  const Real cell_size = 0.1;
  
  const Real start_time = 0.;
  const Real end_time = boost::lexical_cast<Real>(argv[1]);
  const Real dt = boost::lexical_cast<Real>(argv[2]);
  Real t = start_time;
  const Uint write_interval = 5;
  const Real invdt = 1. / dt;
  
  const Real mu = 0.01;
  const Real rho = 1.;
  const Real nu = mu/rho;
  
  RealVector u_inf(2); u_inf << 0.6, 0.;
  RealVector u_wall(2); u_wall.setZero();
  const Real p_out = 0.;
  const Real u_ref = u_inf[XX];
  
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
  Real tau_ps_val; StoredReference<Real> tau_ps = store(tau_ps_val);
  
  URI input_file = URI(argv[4]);
  model.configure_option("input_file", input_file);
  SignalArgs a;
  model.signal_read_mesh(a);
  
  // build up the solver out of different actions
  model
  << model.add_action("InitializePressure", nodes_expression(p = 0.))
  << model.add_action("InitializeVelocity", nodes_expression(u = u_wall)) <<
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
          compute_tau(u, u_ref, nu, tau_ps),
          element_quadrature <<
          (
            _A(p    , u[_i]) +=          transpose(N(p))         * nabla(u)[_i] + tau_ps * transpose(nabla(p)[_i]) * u*nabla(u), // Standard continuity + PSPG for advection
            _A(p    , p)     += tau_ps * transpose(nabla(p))     * nabla(p),     // Continuity, PSPG
            _A(u[_i], u[_i]) += mu     * transpose(nabla(u))     * nabla(u)     + transpose(N(u)) * u*nabla(u),     // Diffusion + advection
            _A(u[_i], p)     += 1./rho * transpose(N(u))         * nabla(p)[_i], // Pressure gradient
            _T(p    , u[_i]) += tau_ps * transpose(nabla(p)[_i]) * N(u),         // Time, PSPG
            _T(u[_i], u[_i]) += transpose(N(u))         * N(u)          // Time, standard
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
    << pr.add_action("IncrementU", nodes_expression(u += pr.solution(u))) // increment solution
    << pr.add_action("IncrementP", nodes_expression(p += pr.solution(p)))
  );

  // Configure timings
  model.time().configure_option("time_step", dt);
  model.time().configure_option("end_time", end_time);
      
  // Run the solver
  model.execute();
  
  // Write result
  URI output_file("navier-stokes-pspg.msh");
  model.configure_option("output_file", output_file);
  model.signal_write_mesh(a);
}

BOOST_AUTO_TEST_SUITE_END()
