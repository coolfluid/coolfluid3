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

inline void 
check_close(const Real a, const Real b, const Real threshold)
{
  BOOST_CHECK_SMALL(a - b, threshold);
}

static boost::proto::terminal< void(*)(Real, Real, Real) >::type const _check_close = {&check_close};

BOOST_AUTO_TEST_SUITE( ProtoSUPGSuite )

/// Stores the coefficients for the SUPG model and shares them inside a proto expression through the state
struct SUPGCoeffs
{
  SUPGCoeffs(Real& p_tau_ps, Real& p_tau_su, Real& p_tau_bulk) :
    tau_ps(p_tau_ps),
    tau_su(p_tau_su),
    tau_bulk(p_tau_bulk)
  {
  }
  
  /// Reference velocity magnitude
  Real u_ref;
  
  /// Kinematic viscosity
  Real nu;
  
  /// Density
  Real rho;
  
  /// Model coefficients
  StoredReference<Real> tau_ps, tau_su, tau_bulk;
};

struct ComputeTau
{ 
  /// Dummy result
  typedef void result_type;
  
  template<typename UT>
  void operator()(const UT& u, SUPGCoeffs& coeffs) const
  {
    const Real he=sqrt(4./3.141592654*u.support().volume());
    const Real ree=coeffs.u_ref*he/(2.*coeffs.nu);
    const Real xi=std::max(0.,std::min(ree/3.,1.));
    coeffs.tau_ps.get() = he*xi/(2.*coeffs.u_ref);
    coeffs.tau_bulk.get() = he*coeffs.u_ref/xi;
    
    // Average cell velocity
    const RealVector2 u_avg = u.value().colwise().mean();
    const Real umag = u_avg.norm();
    coeffs.tau_su.get() = 0.;
    if(umag > 1e-10)
    {
      const Real h = 2. * u.support().volume() / (u.support().nodes() * (u_avg / umag)).array().abs().sum();
      Real ree=umag*h/(2.*coeffs.nu);
      Real xi=std::max(0.,std::min(ree/3.,1.));
      coeffs.tau_su.get() = h*xi/(2.*umag);
    }
  }
};

/// Placeholder for the compute_tau operation
static MakeSFOp<ComputeTau>::type const compute_tau = {};

// Solve the Navier-Stokes equations with SUPG
BOOST_AUTO_TEST_CASE( ProtoNavierStokesSUPG )
{
  int    argc = boost::unit_test::framework::master_test_suite().argc;
  char** argv = boost::unit_test::framework::master_test_suite().argv;

  const Real length = 5.;
  const Real height = 2.;
  const Uint x_segments = 25;
  const Uint y_segments = 10;
  
  const Real start_time = 0.;
  const Real end_time = 50.;
  const Real dt = 5.;
  Real t = start_time;
  const Uint write_interval = 200;
  const Real invdt = 1. / dt;
  
  const Real mu = 0.1;
  const Real rho = 100.;
  
  RealVector u_direction(2); u_direction <<1., 0.;
  RealVector u_wall(2); u_wall << 0., 0.;
  const Real p0 = 5.;
  const Real p1 = 0.;
  const Real c = 0.5 * (p0 - p1) / (rho * mu * length);
  
  // Storage for the calculated coefficients
  Real tau_ps, tau_su, tau_bulk;
  
  SUPGCoeffs coefs(tau_ps, tau_su, tau_bulk);
  coefs.u_ref = c;
  coefs.nu = mu / rho;
  coefs.rho = rho;
  
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
          _A(p    , u[_i]) +=          transpose(N(p))         * nabla(u)[_i] + coefs.tau_ps * transpose(nabla(p)[_i]) * u*nabla(u), // Standard continuity + PSPG for advection
          _A(p    , p)     += coefs.tau_ps * transpose(nabla(p))     * nabla(p),     // Continuity, PSPG
          _A(u[_i], u[_i]) += mu     * transpose(nabla(u))     * nabla(u)     + transpose(N(u) + coefs.tau_su*u*nabla(u)) * u*nabla(u),     // Diffusion + advection
          _A(u[_i], p)     += 1./rho * transpose(N(u) + coefs.tau_su*u*nabla(u)) * nabla(p)[_i], // Pressure gradient (standard and SUPG)
          _T(p    , u[_i]) += coefs.tau_ps * transpose(nabla(p)[_i]) * N(u),         // Time, PSPG
          _T(u[_i], u[_i]) += invdt  * transpose(N(u) + coefs.tau_su*u*nabla(u))         * N(u)          // Time, standard
          ),
          pr.system_matrix += invdt * _T + 1.0 * _A,
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
  URI output_file("navier-stokes-supg.msh");
  model.configure_option("output_file", output_file);
  SignalArgs a;
  model.signal_write_mesh(a);  
}

BOOST_AUTO_TEST_SUITE_END()
