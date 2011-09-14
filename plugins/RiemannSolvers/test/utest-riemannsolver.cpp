// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::RiemannSolvers"

#include <boost/test/unit_test.hpp>

#include "Common/CBuilder.hpp"
#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/OptionComponent.hpp"

#include "RiemannSolvers/RiemannSolver.hpp"
#include "Physics/NavierStokes/Cons2D.hpp"
#include "Physics/NavierStokes/Roe2D.hpp"
#include "Math/Defs.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::RiemannSolvers;
using namespace CF::Physics;
using namespace CF::Physics::NavierStokes;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Physics {

//////////////////////////////////////////////////////////////////////////////

class Physics_API VarTransformer : public Common::Component
{
public:
  typedef boost::shared_ptr<VarTransformer>       Ptr;
  typedef boost::shared_ptr<VarTransformer const> ConstPtr;

  VarTransformer(const std::string& name) : Common::Component(name) {}
  ~VarTransformer() {}
  static std::string type_name() { return "VarTransformer"; }

  virtual void transform(const RealVector& from, RealVector& to) = 0;

  virtual void transform(const Physics::Properties&, RealVector& to) = 0;
};

//////////////////////////////////////////////////////////////////////////////

} // Physics
} // CF

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Physics {
namespace NavierStokes {

//////////////////////////////////////////////////////////////////////////////

class NavierStokes_API Cons2DtoRoe2D : public VarTransformer
{
public:

  typedef boost::shared_ptr<Cons2DtoRoe2D>       Ptr;
  typedef boost::shared_ptr<Cons2DtoRoe2D const> ConstPtr;

  Cons2DtoRoe2D(const std::string& name) : VarTransformer(name) {}
  ~Cons2DtoRoe2D() {}
  static std::string type_name() { return "Cons2DtoRoe2D"; }

  virtual void transform(const RealVector& from, RealVector& to)
  {
    Cons2D::compute_properties(coord,from,grads,p);
    to[0] = sqrt(p.rho);  // sqrt(rho)
    to[1] = to[0]*p.u;    // sqrt(rho)*u
    to[2] = to[0]*p.v;    // sqrt(rho)*v
    to[3] = to[0]*p.H;    // sqrt(rho)*H
  }


  /// @warning QUESTION: WHY CAN THIS FUNCTION NOT BE IN THE VARIABLES API? (caps for visibility)
  virtual void transform(const Physics::Properties& from, RealVector& to)
  {
    const Roe2D::MODEL::Properties& p = *static_cast<Roe2D::MODEL::Properties const*>( &from );
    to[0] = sqrt(p.rho);  // sqrt(rho)
    to[1] = to[0]*p.u;    // sqrt(rho)*u
    to[2] = to[0]*p.v;    // sqrt(rho)*v
    to[3] = to[0]*p.H;    // sqrt(rho)*H
  }

private:
  Cons2D::MODEL::Properties p; //! properties, contains gamma
  Cons2D::MODEL::GeoV coord;   //! dummy
  Cons2D::MODEL::SolM grads;   //! dummy
};

//////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder<Cons2DtoRoe2D,VarTransformer,LibNavierStokes> Cons2DtoRoe2D_builder;

//////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // Physics
} // CF

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RiemannSolvers {

//////////////////////////////////////////////////////////////////////////////

class RiemannSolvers_API Roe : public RiemannSolver
{
public:
  typedef boost::shared_ptr< Roe >       Ptr;
  typedef boost::shared_ptr< Roe const > ConstPtr;

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  /// Contructor
  /// @param name of the component
  Roe ( const std::string& name ) : RiemannSolver(name)
  {
    options().add_option( OptionComponent<Physics::Variables>::create("roe_vars",&m_roe_vars) )
        ->description("The component describing the Roe variables")
        ->pretty_name("Roe Variables");

    options().add_option( OptionComponent<Physics::VarTransformer>::create("solution_to_roe",&m_solution_to_roe) )
        ->description("The component describing the Roe Transformer")
        ->pretty_name("Solution to Roe");

    option("phys_model").attach_trigger( boost::bind( &Roe::trigger_phys_model, this) );
  }

  void trigger_phys_model()
  {
    coord.resize(model().ndim());
    grads.resize(model().neqs(),model().ndim());
    p_left  = model().create_properties();
    p_right = model().create_properties();
    p_avg   = model().create_properties();

    roe_left.resize(model().neqs());
    roe_right.resize(model().neqs());
    roe_avg.resize(model().neqs());

    f_left.resize(model().neqs(),model().ndim());
    f_right.resize(model().neqs(),model().ndim());

    eigenvalues.resize(model().neqs());
    right_eigenvectors.resize(model().neqs(),model().neqs());
    left_eigenvectors.resize(model().neqs(),model().neqs());
    abs_jacobian.resize(model().neqs(),model().neqs());
  }

  /// Virtual destructor
  virtual ~Roe() {};

public:

  virtual void compute_interface_flux(const RealVector& left, const RealVector& right, const RealVector& normal,
                                      RealVector& flux, Real& wave_speed)
  {
    // Compute left and right properties
    solution_vars().compute_properties(coord,left,grads,*p_left);
    solution_vars().compute_properties(coord,right,grads,*p_right);

    // Compute the Roe averaged properties
    // Roe-average = standard average of the Roe-parameter vectors
    solution_to_roe().transform(*p_left, roe_left);    // left Roe parameter vector
    solution_to_roe().transform(*p_right,roe_right);  // right Roe paramter vector
    roe_avg = 0.5*(roe_left+roe_right);             // Roe-average is result
    roe_vars().compute_properties(coord, roe_avg, grads, *p_avg);

    // Compute absolute jacobian using Roe averaged properties
    solution_vars().flux_jacobian_eigen_structure(*p_avg,normal,right_eigenvectors,left_eigenvectors,eigenvalues);
    abs_jacobian = right_eigenvectors * eigenvalues.cwiseAbs().asDiagonal() * left_eigenvectors;

    // Compute left and right fluxes
    solution_vars().flux(*p_left , f_left);
    solution_vars().flux(*p_right, f_right);

    // Compute flux at interface composed of central part and upwind part
    flux = 0.5*(f_left*normal+f_right*normal) - 0.5 * abs_jacobian*(right-left);
  }

private:

  Physics::Variables& roe_vars() { return *m_roe_vars.lock(); }
  Physics::VarTransformer& solution_to_roe() { return *m_solution_to_roe.lock(); }

  boost::weak_ptr<Physics::VarTransformer> m_solution_to_roe;
  boost::weak_ptr<Physics::Variables> m_roe_vars;
  std::auto_ptr<Physics::Properties> p_left;
  std::auto_ptr<Physics::Properties> p_right;
  std::auto_ptr<Physics::Properties> p_avg;
  RealVector coord;
  RealMatrix grads;
  RealMatrix f_left;
  RealMatrix f_right;
  RealVector roe_left;
  RealVector roe_right;
  RealVector roe_avg;
  RealVector eigenvalues;
  RealMatrix left_eigenvectors;
  RealMatrix right_eigenvectors;
  RealMatrix abs_jacobian;
};

//////////////////////////////////////////////////////////////////////////////

} // RiemannSolvers
} // CF

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( RiemannSolvers_Suite )

BOOST_AUTO_TEST_CASE( dynamic_version )
{
  Component& model =  Core::instance().root().create_component<Component>("model");

  // Creation of physics + variables
  PhysModel& physics = model.create_component("navierstokes","CF.Physics.NavierStokes.NavierStokes2D").as_type<PhysModel>();
  Variables& sol_vars = *physics.create_variables("Cons2D","solution");
  Variables& roe_vars = *physics.create_variables("Roe2D","roe");
  VarTransformer& solution_to_roe = physics.create_component("solution_to_roe","CF.Physics.NavierStokes.Cons2DtoRoe2D").as_type<VarTransformer>();

  // Creation + configuration of riemann solver
  RiemannSolver& riemann = model.create_component<Roe>("riemann").as_type<RiemannSolver>();
  riemann.configure_option("phys_model",physics.uri());
  riemann.configure_option("solution_vars",sol_vars.uri());
  riemann.configure_option("roe_vars",roe_vars.uri());
  riemann.configure_option("solution_to_roe",solution_to_roe.uri());

  // Check configuration
  BOOST_CHECK_EQUAL(sol_vars.uri().string() , riemann.solution_vars().uri().string());
  BOOST_CHECK_EQUAL(sol_vars.description().description(),"Rho[1],RhoU[2],RhoE[1]");
  BOOST_CHECK_EQUAL(roe_vars.description().description(),"Z0[1],Z1[1],Z2[1],Z3[1]");

  std::cout << model.tree() << std::endl;


  // Check simple flux computation
  RealVector normal(2);
  RealVector left(4), right(4);
  RealVector flux(4);

  const Real r_L = 4.696;     const Real r_R = 1.408;
  const Real u_L = 0.;        const Real u_R = 0.;
  const Real v_L = 0.;        const Real v_R = 0.;
  const Real p_L = 404400;    const Real p_R = 101100;
  const Real g = 1.4;

  normal << 1. , 0.;
  left <<  r_L, r_L*u_L, r_L*v_L, p_L/(g-1.) + 0.5*r_L*(u_L*u_L+v_L*v_L);
  right << r_R, r_R*u_R, r_L*v_L, p_R/(g-1.) + 0.5*r_R*(u_R*u_R+v_R*v_R);

  Real wavespeed;
  riemann.compute_interface_flux(left,right, normal, flux, wavespeed);

  const Real tol (0.000001);
  BOOST_CHECK_CLOSE(flux[0] , 450.190834 , tol);
  BOOST_CHECK_CLOSE(flux[1] , 252750 , tol);
  BOOST_CHECK_CLOSE(flux[2] , 0      , tol);
  BOOST_CHECK_CLOSE(flux[3] , 127710965.918678 , tol);
}

#if 0
//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_Roe_adv_diff )
{
  RiemannSolver& riemannsolver = Core::instance().root().create_component("Roe-solver-AdvectionDiffusion1D","CF.RiemannSolvers.Roe").as_type<RiemannSolver>();

  Component& state = Core::instance().root().create_component("solution-state-AdvectionDiffusion1D","CF.AdvectionDiffusion.State1D");
  riemannsolver.configure_option("solution_state",state.uri());
  riemannsolver.configure_option("roe_state",std::string("CF.AdvectionDiffusion.State1D"));

  RealVector left(1);   left   << 1.5;
  RealVector right(1);  right  << 0.5;
  RealVector normal(1); normal << 1.;
  RealVector flux(1);
  Real left_wave_speed;
  Real right_wave_speed;
  riemannsolver.solve(left,right,normal,
                      flux,left_wave_speed,right_wave_speed);

  BOOST_CHECK_EQUAL( flux[0] , 1.5 );
  BOOST_CHECK_EQUAL( left_wave_speed  ,  1. );
  BOOST_CHECK_EQUAL( right_wave_speed , -1. );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_Roe_euler1d )
{
  RiemannSolver& riemannsolver = Core::instance().root().create_component("Roe-solver-Euler1D","CF.RiemannSolvers.Roe").as_type<RiemannSolver>();

  Solver::State& state = Core::instance().root().create_component("solution-state-Euler1D","CF.Euler.Cons1D").as_type<Solver::State>();
  riemannsolver.configure_option("solution_state",state.uri());
  riemannsolver.configure_option("roe_state",std::string("CF.Euler.Roe1D"));

  RealVector left(3);
  RealVector right(3);

  Real g=1.4;

  const Real r_L = 4.696;     const Real r_R = 1.408;
  const Real u_L = 0.;        const Real u_R = 0.;
  const Real p_L = 404400;    const Real p_R = 101100;

  left <<  r_L, r_L*u_L, p_L/(g-1.) + 0.5*r_L*u_L*u_L;
  right << r_R, r_R*u_R, p_R/(g-1.) + 0.5*r_R*u_R*u_R;

  RealVector normal(1); normal << 1.;
  RealVector flux(3);
  Real left_wave_speed;
  Real right_wave_speed;
  riemannsolver.solve(left,right,normal,
                      flux,left_wave_speed,right_wave_speed);

  const Real tol (0.000001);
  BOOST_CHECK_CLOSE(flux[0] , 450.190834 , tol);
  BOOST_CHECK_CLOSE(flux[1] , 252750 , tol);
  BOOST_CHECK_CLOSE(flux[2] , 127710965.918678 , tol);

  riemannsolver.solve(right,left,-normal,
                      flux,right_wave_speed,left_wave_speed);

  BOOST_CHECK_CLOSE(flux[0] , -450.190834 , tol);
  BOOST_CHECK_CLOSE(flux[1] , -252750 , tol);
  BOOST_CHECK_CLOSE(flux[2] , -127710965.918678 , tol);

}

BOOST_AUTO_TEST_CASE( test_Roe_euler2d )
{
  RiemannSolver& riemannsolver = Core::instance().root().create_component("Roe-solver-Euler2D","CF.RiemannSolvers.Roe").as_type<RiemannSolver>();

  Solver::State& state = Core::instance().root().create_component("solution-state-Euler2D","CF.Euler.Cons2D").as_type<Solver::State>();
  riemannsolver.configure_option("solution_state",state.uri());
  riemannsolver.configure_option("roe_state",std::string("CF.Euler.Roe2D"));

  RealVector left(4);
  RealVector right(4);
  RealVector flux(4);
  Real left_wave_speed;
  Real right_wave_speed;
  RealVector normal(2);
  const Real tol (0.000001);

  Real g=1.4;

  const Real r_L = 4.696;     const Real r_R = 1.408;
  const Real u_L = 0.;        const Real u_R = 0.;
  const Real v_L = 0.;        const Real v_R = 0.;
  const Real p_L = 404400;    const Real p_R = 101100;

  left <<  r_L, r_L*u_L, r_L*v_L, p_L/(g-1.) + 0.5*r_L*(u_L*u_L+v_L*v_L);
  right << r_R, r_R*u_R, r_R*v_R, p_R/(g-1.) + 0.5*r_R*(u_R*u_R+v_R*v_R);


  normal << 1. , 0.;
  riemannsolver.solve(left,right,normal,
                      flux,left_wave_speed,right_wave_speed);
  BOOST_CHECK_CLOSE(flux[0] , 450.190834 , tol);
  BOOST_CHECK_CLOSE(flux[1] , 252750 , tol);
  BOOST_CHECK_CLOSE(flux[2]+1. , 1. , tol);
  BOOST_CHECK_CLOSE(flux[3] , 127710965.918678 , tol);
  BOOST_CHECK_CLOSE(left_wave_speed, 336.8571471643333 , tol);

  riemannsolver.solve(right,left,-normal,
                      flux,right_wave_speed,left_wave_speed);
  BOOST_CHECK_CLOSE(flux[0] , -450.190834 , tol);
  BOOST_CHECK_CLOSE(flux[1] , -252750 , tol);
  BOOST_CHECK_CLOSE(flux[2]+1. , 1. , tol);
  BOOST_CHECK_CLOSE(flux[3] , -127710965.918678 , tol);
  BOOST_CHECK_CLOSE(left_wave_speed, 336.8571471643333 , tol);

  normal << 0., 1.;
  riemannsolver.solve(left,right,normal,
                      flux,left_wave_speed,right_wave_speed);
  BOOST_CHECK_CLOSE(flux[0] , 450.190834 , tol);
  BOOST_CHECK_CLOSE(flux[1]+1. , 1. , tol);
  BOOST_CHECK_CLOSE(flux[2] , 252750 , tol);
  BOOST_CHECK_CLOSE(flux[3] , 127710965.918678 , tol);
  BOOST_CHECK_CLOSE(left_wave_speed, 336.8571471643333 , tol);

  riemannsolver.solve(right,left,-normal,
                      flux,right_wave_speed,left_wave_speed);
  BOOST_CHECK_CLOSE(flux[0] , -450.190834 , tol);
  BOOST_CHECK_CLOSE(flux[1]+1. , 1. , tol);
  BOOST_CHECK_CLOSE(flux[2] , -252750 , tol);
  BOOST_CHECK_CLOSE(flux[3] , -127710965.918678 , tol);
  BOOST_CHECK_CLOSE(left_wave_speed, 336.8571471643333 , tol);

  normal << 1., 1.;
  riemannsolver.solve(left,right,normal,
                      flux,left_wave_speed,right_wave_speed);
  BOOST_CHECK_CLOSE(flux[1] , flux[2] , tol);

  riemannsolver.solve(left,right,-normal,
                      flux,left_wave_speed,right_wave_speed);
  BOOST_CHECK_CLOSE(flux[1] , flux[2] , tol);
  BOOST_CHECK_CLOSE(flux[1] , flux[2] , tol);
  BOOST_CHECK_CLOSE(left_wave_speed, 336.8571471643333 , tol);
}
#endif
////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

