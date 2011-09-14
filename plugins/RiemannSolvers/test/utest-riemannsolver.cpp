// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::RiemannSolvers"

#include <boost/test/unit_test.hpp>


#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"

//#include "Solver/State.hpp"
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

template <typename FROM_VAR, typename TO_VAR>
class Transform
{
public:
  template <typename VecT1, typename VecT2>
  void operator()(const VecT1& from, VecT2& to) const
  {
    FROM_VAR::compute_properties(coord, from , grads, p_from);
    TO_VAR::compute_variables(p_from, to);
  }

  static typename FROM_VAR::MODEL::Properties p_from;

  // just here because compute_properties needs it
  static const typename FROM_VAR::MODEL::GeoV coord;
  static const typename FROM_VAR::MODEL::SolM grads;
};

template <typename FROM_VAR, typename TO_VAR>
typename FROM_VAR::MODEL::Properties Transform<FROM_VAR,TO_VAR>::p_from;
template <typename FROM_VAR, typename TO_VAR>
const typename FROM_VAR::MODEL::GeoV Transform<FROM_VAR,TO_VAR>::coord;
template <typename FROM_VAR, typename TO_VAR>
const typename FROM_VAR::MODEL::SolM Transform<FROM_VAR,TO_VAR>::grads;

//////////////////////////////////////////////////////////////////////////////

template <typename SOLVAR, typename ROEVAR>
class Roe : public RiemannSolverT< Roe<SOLVAR,ROEVAR> >
{
public:
  typedef boost::shared_ptr< Roe >       Ptr;
  typedef boost::shared_ptr< Roe const > ConstPtr;

public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  /// Contructor
  /// @param name of the component
  Roe ( const std::string& name ) : RiemannSolverT<Roe<SOLVAR,ROEVAR> >(name)
  {
    BOOST_STATIC_ASSERT(sizeof(typename SOLVAR::MODEL) == sizeof(typename ROEVAR::MODEL));
  }

  /// Virtual destructor
  virtual ~Roe() {};

public:

  template <typename SolT, typename FluxT, typename CoordsT>
  static void compute_interface_flux(const SolT& left, const SolT& right, const CoordsT& normal,
                                     FluxT& flux, Real& wave_speed)
  {
    // Compute left and right flux
    SOLVAR::compute_properties(coord, left,  grads, p_left);
    SOLVAR::compute_properties(coord, right, grads, p_right);
    SOLVAR::flux(p_left, f_left );
    SOLVAR::flux(p_right,f_right);

    // Compute roe average
    transform_to_roe(left,roe_left);
    transform_to_roe(right,roe_right);
    roe_avg = 0.5*(roe_left + roe_right);
    ROEVAR::compute_properties(coord, roe_avg, grads, p_avg);

    // Compute absolute jacobian using roe averaged properties
    SOLVAR::flux_jacobian_eigen_structure(p_avg,normal,right_eigenvectors,left_eigenvectors,eigenvalues);
    abs_jacobian = right_eigenvectors * eigenvalues.cwiseAbs().asDiagonal() * left_eigenvectors;

    // Compute flux at interface composed of central part and upwind part
    flux = 0.5*(f_left*normal+f_right*normal) - 0.5 * abs_jacobian*(right-left);
  }

private:
  static const Transform<SOLVAR,ROEVAR> transform_to_roe;
  static typename SOLVAR::MODEL::Properties p_left;
  static typename SOLVAR::MODEL::Properties p_right;
  static typename SOLVAR::MODEL::Properties p_avg;
  static const typename SOLVAR::MODEL::GeoV coord;
  static const typename SOLVAR::MODEL::SolM grads;
  static typename SOLVAR::MODEL::SolM f_left;
  static typename SOLVAR::MODEL::SolM f_right;
  static typename SOLVAR::MODEL::SolV roe_left;
  static typename SOLVAR::MODEL::SolV roe_right;
  static typename SOLVAR::MODEL::SolV roe_avg;
  static Eigen::Matrix<Real, SOLVAR::MODEL::_neqs, 1> eigenvalues;
  static Eigen::Matrix<Real, SOLVAR::MODEL::_neqs, SOLVAR::MODEL::_neqs> left_eigenvectors;
  static Eigen::Matrix<Real, SOLVAR::MODEL::_neqs, SOLVAR::MODEL::_neqs> right_eigenvectors;
  static Eigen::Matrix<Real, SOLVAR::MODEL::_neqs, SOLVAR::MODEL::_neqs> abs_jacobian;
};

template <typename SOLVAR, typename ROEVAR>
const Transform<SOLVAR,ROEVAR> Roe<SOLVAR,ROEVAR>::transform_to_roe;
template <typename SOLVAR, typename ROEVAR>
typename SOLVAR::MODEL::Properties Roe<SOLVAR,ROEVAR>::p_left;
template <typename SOLVAR, typename ROEVAR>
typename SOLVAR::MODEL::Properties Roe<SOLVAR,ROEVAR>::p_right;
template <typename SOLVAR, typename ROEVAR>
typename SOLVAR::MODEL::Properties Roe<SOLVAR,ROEVAR>::p_avg;
template <typename SOLVAR, typename ROEVAR>
const typename SOLVAR::MODEL::GeoV Roe<SOLVAR,ROEVAR>::coord;
template <typename SOLVAR, typename ROEVAR>
const typename SOLVAR::MODEL::SolM Roe<SOLVAR,ROEVAR>::grads;
template <typename SOLVAR, typename ROEVAR>
typename SOLVAR::MODEL::SolM Roe<SOLVAR,ROEVAR>::f_left;
template <typename SOLVAR, typename ROEVAR>
typename SOLVAR::MODEL::SolM Roe<SOLVAR,ROEVAR>::f_right;
template <typename SOLVAR, typename ROEVAR>
typename SOLVAR::MODEL::SolV Roe<SOLVAR,ROEVAR>::roe_left;
template <typename SOLVAR, typename ROEVAR>
typename SOLVAR::MODEL::SolV Roe<SOLVAR,ROEVAR>::roe_right;
template <typename SOLVAR, typename ROEVAR>
typename SOLVAR::MODEL::SolV Roe<SOLVAR,ROEVAR>::roe_avg;
template <typename SOLVAR, typename ROEVAR>
Eigen::Matrix<Real, SOLVAR::MODEL::_neqs, 1> Roe<SOLVAR,ROEVAR>::eigenvalues;
template <typename SOLVAR, typename ROEVAR>
Eigen::Matrix<Real, SOLVAR::MODEL::_neqs, SOLVAR::MODEL::_neqs> Roe<SOLVAR,ROEVAR>::left_eigenvectors;
template <typename SOLVAR, typename ROEVAR>
Eigen::Matrix<Real, SOLVAR::MODEL::_neqs, SOLVAR::MODEL::_neqs> Roe<SOLVAR,ROEVAR>::right_eigenvectors;
template <typename SOLVAR, typename ROEVAR>
Eigen::Matrix<Real, SOLVAR::MODEL::_neqs, SOLVAR::MODEL::_neqs> Roe<SOLVAR,ROEVAR>::abs_jacobian;

BOOST_AUTO_TEST_SUITE( RiemannSolvers_Suite )

BOOST_AUTO_TEST_CASE( static_version )
{
  RealVector2 normal;
  RealVector4 left, right;
  RealVector4 flux;

  const Real r_L = 4.696;     const Real r_R = 1.408;
  const Real u_L = 0.;        const Real u_R = 0.;
  const Real v_L = 0.;        const Real v_R = 0.;
  const Real p_L = 404400;    const Real p_R = 101100;
  const Real g = 1.4;

  normal << 1. , 0.;
  left <<  r_L, r_L*u_L, r_L*v_L, p_L/(g-1.) + 0.5*r_L*(u_L*u_L+v_L*v_L);
  right << r_R, r_R*u_R, r_L*v_L, p_R/(g-1.) + 0.5*r_R*(u_R*u_R+v_R*v_R);


  Real wavespeed;
  Roe<NavierStokes::Cons2D,NavierStokes::Roe2D>::compute_interface_flux(left,right, normal, flux, wavespeed);
}

BOOST_AUTO_TEST_CASE( dynamic_version )
{
  Component& root = Core::instance().root();
  Component& model = root.create_component<Component>("model");
  PhysModel& physics = model.create_component("navierstokes","CF.Physics.NavierStokes.NavierStokes2D").as_type<PhysModel>();
  Variables& vars = *physics.create_variables("Cons2D","solution");
  RiemannSolver& riemann = model.create_component< Roe<NavierStokes::Cons2D,NavierStokes::Roe2D> >("riemann").as_type<RiemannSolver>();
  riemann.configure_option("solution_vars",vars.uri());
  BOOST_CHECK_EQUAL(vars.uri().string() , riemann.sol_vars().uri().string());
//  BOOST_CHECK_EQUAL(vars.description().description(),"Rho[1],RhoU[2],RhoE[1]");
  std::cout << model.tree() << std::endl;
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
  RiemannSolver::Ptr roe_solver = allocate_component< Roe<NavierStokes::Cons2D,NavierStokes::Roe2D> >("roe-solver");
  roe_solver->compute_interface_flux(left,right, normal, flux, wavespeed);

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

