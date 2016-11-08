// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_semi_implicit_PressureMatrixAssembly_hpp
#define cf3_UFEM_semi_implicit_PressureMatrixAssembly_hpp

#include "NavierStokesSemiImplicit.hpp"

#include <boost/mpl/back_inserter.hpp>
#include <boost/mpl/copy.hpp>

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

using boost::proto::lit;

/// Helper to detemine the appropriate default integration order
template<typename ElementT>
struct IntegralOrder
{
  const static int value = 2;
};

/// Triangles get order 1
template<>
struct IntegralOrder<mesh::LagrangeP1::Triag2D>
{
  const static int value = 1;
};

/// Tetrahedra get order 1
template<>
struct IntegralOrder<mesh::LagrangeP1::Tetra3D>
{
  const static int value = 1;
};

struct VelocityAssembly
{
  typedef void result_type;

  /// Compute the coefficients for the full Navier-Stokes equations
  template<typename UT, typename UAdvT, typename NUT, typename MatrixT>
  void operator()(const UT& u_fd, const UAdvT& u, const NUT& nu_eff, MatrixT& M, MatrixT& T, const Real& dt, const Real& tau_bulk) const
  {
    typedef typename UT::EtypeT ElementT;

    static const Uint nb_nodes = ElementT::nb_nodes;
    static const Uint dim = ElementT::dimension;

    Eigen::Matrix<Real, nb_nodes, nb_nodes> laplacian;
    Eigen::Matrix<Real, nb_nodes, nb_nodes> su_N;
    Eigen::Matrix<Real, nb_nodes, nb_nodes> bulk_block;

    static const int ideal_order = IntegralOrder<ElementT>::value;

    // Always use second order for the mass part
    if(ideal_order != 2)
    {
      typedef mesh::Integrators::GaussMappedCoords<2, ElementT::shape> Gauss2T;
      for(Uint gauss_idx = 0; gauss_idx != Gauss2T::nb_points; ++gauss_idx)
      {
        // This precomputes the required matrix operators
        u.support().compute_shape_functions(Gauss2T::instance().coords.col(gauss_idx));
        u.support().compute_jacobian(Gauss2T::instance().coords.col(gauss_idx));
        u.compute_values(Gauss2T::instance().coords.col(gauss_idx));

        const Real w = Gauss2T::instance().weights[gauss_idx] * u.support().jacobian_determinant();

        su_N = (w * (u.shape_function()).transpose())*u.shape_function();

        for(Uint i = 0; i != dim; ++i)
        {
          T.template block<nb_nodes, nb_nodes>(i*nb_nodes, i*nb_nodes) += su_N;
        }
      }
    }

    typedef mesh::Integrators::GaussMappedCoords<ideal_order, ElementT::shape> GaussT;

    for(Uint gauss_idx = 0; gauss_idx != GaussT::nb_points; ++gauss_idx)
    {
      // This precomputes the required matrix operators
      u.support().compute_shape_functions(GaussT::instance().coords.col(gauss_idx));
      u.support().compute_jacobian(GaussT::instance().coords.col(gauss_idx));
      u.compute_values(GaussT::instance().coords.col(gauss_idx));
      nu_eff.compute_values(GaussT::instance().coords.col(gauss_idx));

      const Real w = GaussT::instance().weights[gauss_idx] * u.support().jacobian_determinant();

      const Real bulk_coeff = w*tau_bulk;
      laplacian = w*nu_eff.eval()*(u.nabla().transpose()*u.nabla()); // laplacian operator
      if(ideal_order == 2)
      {
        su_N = (w * (u.shape_function()).transpose())*u.shape_function();
        for(Uint i = 0; i != dim; ++i)
        {
          M.template block<nb_nodes, nb_nodes>(i*nb_nodes, i*nb_nodes) += laplacian + bulk_coeff * u.nabla().row(i).transpose()*u.nabla().row(i);
          T.template block<nb_nodes, nb_nodes>(i*nb_nodes, i*nb_nodes) += su_N;
        }
      }
      else
      {
        for(Uint i = 0; i != dim; ++i)
        {
          M.template block<nb_nodes, nb_nodes>(i*nb_nodes, i*nb_nodes) += laplacian + bulk_coeff * u.nabla().row(i).transpose()*u.nabla().row(i);
        }
      }
    }
    
    // Bulk terms are OK with 1st order inegration
    typedef mesh::Integrators::GaussMappedCoords<1, ElementT::shape> Gauss1T;

    const Uint gauss_idx = 0;
    // This precomputes the required matrix operators
    u.support().compute_shape_functions(Gauss1T::instance().coords.col(gauss_idx));
    u.support().compute_jacobian(Gauss1T::instance().coords.col(gauss_idx));
    u.compute_values(Gauss1T::instance().coords.col(gauss_idx));
    nu_eff.compute_values(Gauss1T::instance().coords.col(gauss_idx));

    const Real w = Gauss1T::instance().weights[gauss_idx] * u.support().jacobian_determinant();
    const Real bulk_coeff = w*tau_bulk;
    
    for(Uint i = 0; i != dim; ++i)
    {
      for(Uint j = i+1; j != dim; ++j)
      {
        bulk_block = bulk_coeff * u.nabla().row(i).transpose()*u.nabla().row(j);
        M.template block<nb_nodes, nb_nodes>(i*nb_nodes, j*nb_nodes) += bulk_block;
        M.template block<nb_nodes, nb_nodes>(j*nb_nodes, i*nb_nodes) += bulk_block.transpose();
      }
    }
  }
};

static solver::actions::Proto::MakeSFOp<VelocityAssembly>::type const velocity_assembly = {};

struct PressureRHS
{
  template<typename Signature>
  struct result;

  template<typename This, typename UT, typename UVecT, typename PVecT>
  struct result<This(UT, UVecT, UVecT, UVecT, PVecT, Real, Real)>
  {
    typedef const PVecT& type;
  };

  /// Compute the coefficients for the full Navier-Stokes equations
  template<typename UT, typename UVecT, typename PVecT>
  const PVecT& operator()(PVecT& result, const UT& u, const UVecT& u_vec, const UVecT& a, const UVecT& delta_a, const PVecT& p_vec, const Real& tau_ps, const Real& dt) const
  {
    typedef typename UT::EtypeT ElementT;

    static const Uint nb_nodes = ElementT::nb_nodes;
    static const Uint dim = ElementT::dimension;

    Eigen::Matrix<Real, 1, nb_nodes> adv;
    Eigen::Matrix<Real, nb_nodes, nb_nodes> App;
    const UVecT u_plus_dt_da = u_vec + dt*delta_a;
    const UVecT a_plus_da = a + delta_a;

    App.setZero();
    result.setZero();

    typedef mesh::Integrators::GaussMappedCoords<IntegralOrder<ElementT>::value, ElementT::shape> GaussT;

    for(Uint gauss_idx = 0; gauss_idx != GaussT::nb_points; ++gauss_idx)
    {
      // This precomputes the required matrix operators
      u.support().compute_shape_functions(GaussT::instance().coords.col(gauss_idx));
      u.support().compute_jacobian(GaussT::instance().coords.col(gauss_idx));
      u.compute_values(GaussT::instance().coords.col(gauss_idx));

      const Real w = GaussT::instance().weights[gauss_idx] * u.support().jacobian_determinant();
      const Real tau_w = w*tau_ps;

      adv = tau_ps*u.eval() * u.nabla(); // advection operator
      App += tau_w * u.nabla().transpose() * u.nabla();
      for(Uint i = 0; i != dim; ++i)
      {
        const Real a = w*(u.nabla().row(i)*u_plus_dt_da.row(i).transpose())[0];
        const Real b = w*(adv*u_plus_dt_da.row(i).transpose())[0];
        const Real c = tau_w*(u.shape_function()*a_plus_da.row(i).transpose())[0];
        
        result -= a*(u.shape_function() + adv*0.5).transpose() + (b+c)*u.nabla().row(i).transpose();
      }
    }

    result -= App*p_vec;

    return result;
  }
};

static solver::actions::Proto::MakeSFOp<PressureRHS>::type const pressure_rhs = {};

struct VelocityRHS
{
  template<typename Signature>
  struct result;

  template<typename This, typename UT, typename NUT, typename UVecT1, typename UVecT2, typename PVecT>
  struct result<This(UT, NUT, UVecT1, UVecT2, PVecT, Real, Real)>
  {
    typedef const Eigen::Matrix<Real, UT::EtypeT::nb_nodes*UT::EtypeT::dimension, 1>& type;
  };

  template<typename This, typename UT, typename NUT, typename GT, typename UVecT1, typename UVecT2, typename PVecT>
  struct result<This(UT, NUT, GT, UVecT1, UVecT2, PVecT, Real, Real)>
  {
    typedef const Eigen::Matrix<Real, UT::EtypeT::nb_nodes*UT::EtypeT::dimension, 1>& type;
  };

  /// Compute the coefficients for the full Navier-Stokes equations
  template<typename StorageT, typename UT, typename NUT, typename UVecT1, typename UVecT2, typename PVecT>
  const StorageT& operator()(StorageT& result, const UT& u, const NUT& nu_eff, const UVecT1& a_vec, const UVecT2& dt_a_min_u_in, const PVecT& p_plus_dp_in, const Real& tau_su, const Real& tau_bulk) const
  {
    typedef typename UT::EtypeT ElementT;

    static const Uint nb_nodes = ElementT::nb_nodes;
    static const Uint dim = ElementT::dimension;

    Eigen::Matrix<Real, 1, nb_nodes> adv;
    Eigen::Matrix<Real, 1, nb_nodes> N_plus_adv;
    const Eigen::Matrix<Real, nb_nodes, 1> p_plus_dp = p_plus_dp_in;
    const UVecT1 dt_a_min_u = dt_a_min_u_in;
    
    result.setZero();

    static const int ideal_order = IntegralOrder<ElementT>::value;

    if(ideal_order != 2)
    {
      /// Mass matrix part is always second order
      typedef mesh::Integrators::GaussMappedCoords<2, ElementT::shape> Gauss2T;
      for(Uint gauss_idx = 0; gauss_idx != Gauss2T::nb_points; ++gauss_idx)
      {
        // This precomputes the required matrix operators
        u.support().compute_shape_functions(Gauss2T::instance().coords.col(gauss_idx));
        u.support().compute_jacobian(Gauss2T::instance().coords.col(gauss_idx));
        u.compute_values(Gauss2T::instance().coords.col(gauss_idx));

        const Real w = Gauss2T::instance().weights[gauss_idx] * u.support().jacobian_determinant();

        adv = u.eval() * u.nabla(); // advection operator
        N_plus_adv = tau_su*adv + u.shape_function();

        // Compose the bulk  and skew symmetric term
        Real f = 0.;
        for(Uint i = 0; i != dim; ++i)
        {
          f += u.nabla().row(i)*dt_a_min_u.row(i).transpose();
        }
        f *= w;
        const Real f_skew = 0.5*f;

        for(Uint i = 0; i != dim; ++i)
        {
          const Real c = -w*(u.shape_function()*(a_vec.row(i)).transpose())[0];
          const Real e = w*adv*dt_a_min_u.row(i).transpose();

          result.template segment<nb_nodes>(i*nb_nodes) += N_plus_adv.transpose() * (c + e + (f_skew * u.eval()[i]));
        }
      }
    }

    typedef mesh::Integrators::GaussMappedCoords<ideal_order, ElementT::shape> GaussT;

    for(Uint gauss_idx = 0; gauss_idx != GaussT::nb_points; ++gauss_idx)
    {
      // This precomputes the required matrix operators
      u.support().compute_shape_functions(GaussT::instance().coords.col(gauss_idx));
      u.support().compute_jacobian(GaussT::instance().coords.col(gauss_idx));
      u.compute_values(GaussT::instance().coords.col(gauss_idx));
      nu_eff.compute_values(GaussT::instance().coords.col(gauss_idx));

      const Real w = GaussT::instance().weights[gauss_idx] * u.support().jacobian_determinant();
      const Real w_visc = w * nu_eff.eval();
      const Real tau_su_w = w * tau_su;

      adv = u.eval() * u.nabla(); // advection operator
      const Real b = -w*(u.shape_function()*p_plus_dp)[0];
      
      // Compose the bulk  and skew symmetric term
      Real f = 0.;
      for(Uint i = 0; i != dim; ++i)
      {
        f += u.nabla().row(i)*dt_a_min_u.row(i).transpose();
      }
      f *= w;
      const Real f_bulk = tau_bulk*f;
      if(ideal_order == 2)
      {
        N_plus_adv = tau_su*adv + u.shape_function();
        const Real f_skew = 0.5*f;
        for(Uint i = 0; i != dim; ++i)
        {
          const Real a = tau_su_w*(u.nabla().row(i)*p_plus_dp)[0];
          const Real c = -w*(u.shape_function()*(a_vec.row(i)).transpose())[0];
          const Eigen::Matrix<Real, dim, 1> d = w_visc*(u.nabla()*dt_a_min_u.row(i).transpose());
          const Real e = w*adv*dt_a_min_u.row(i).transpose();


          result.template segment<nb_nodes>(i*nb_nodes) += adv.transpose() * a
            + u.nabla().row(i).transpose() * (b + f_bulk)
            + N_plus_adv.transpose() * (c + e + (f_skew * u.eval()[i]))
            + u.nabla().transpose() * d;
        }
      }
      else
      {
        for(Uint i = 0; i != dim; ++i)
        {
          const Real a = tau_su_w*(u.nabla().row(i)*p_plus_dp)[0];
          const Eigen::Matrix<Real, dim, 1> d = w_visc*(u.nabla()*dt_a_min_u.row(i).transpose());

          result.template segment<nb_nodes>(i*nb_nodes) += adv.transpose() * a
            + u.nabla().row(i).transpose() * (b + f_bulk)
            + u.nabla().transpose() * d;
        }
      }
    }

    return result;
  }

  template<typename StorageT, typename UT, typename NUT, typename GT, typename UVecT1, typename UVecT2, typename PVecT>
  const StorageT& operator()(StorageT& result, const UT& u, const NUT& nu_eff, const GT& g, const UVecT1& a_vec, const UVecT2& dt_a_min_u_in, const PVecT& p_plus_dp_in, const Real& tau_su, const Real& tau_bulk) const
  {
    typedef typename UT::EtypeT ElementT;
    static const Uint nb_nodes = ElementT::nb_nodes;
    static const Uint dim = ElementT::dimension;

    (*this)(result, u, nu_eff, a_vec, dt_a_min_u_in, p_plus_dp_in, tau_su, tau_bulk);
    typedef mesh::Integrators::GaussMappedCoords<2, ElementT::shape> Gauss2T;
    for(Uint gauss_idx = 0; gauss_idx != Gauss2T::nb_points; ++gauss_idx)
    {
      // This precomputes the required matrix operators
      g.support().compute_shape_functions(Gauss2T::instance().coords.col(gauss_idx));
      g.support().compute_jacobian(Gauss2T::instance().coords.col(gauss_idx));
      g.compute_values(Gauss2T::instance().coords.col(gauss_idx));

      const Real w = Gauss2T::instance().weights[gauss_idx] * g.support().jacobian_determinant();

      for(Uint i = 0; i != dim; ++i)
      {
        result.template segment<nb_nodes>(i*nb_nodes) += g.shape_function().transpose() * g.eval()[i] * w;
      }
    }
    return result;
  }
};

static solver::actions::Proto::MakeSFOp<VelocityRHS>::type const velocity_rhs = {};

struct ApplyAup
{
  template<typename Signature>
  struct result;

  template<typename This, typename UT, typename PVecT>
  struct result<This(UT, PVecT, Real, Real)>
  {
    typedef const Eigen::Matrix<Real, UT::EtypeT::nb_nodes*UT::EtypeT::dimension, 1>& type;
  };

  /// Compute the coefficients for the full Navier-Stokes equations
  template<typename StorageT, typename UT, typename PVecT>
  const StorageT& operator()(StorageT& result, const UT& u, const PVecT& delta_p, const Real& tau_su, const Real& theta) const
  {
    typedef typename UT::EtypeT ElementT;

    static const Uint nb_nodes = ElementT::nb_nodes;
    static const Uint dim = ElementT::dimension;

    Eigen::Matrix<Real, 1, nb_nodes> adv;
    
    result.setZero();

    typedef mesh::Integrators::GaussMappedCoords<IntegralOrder<ElementT>::value, ElementT::shape> GaussT;

    for(Uint gauss_idx = 0; gauss_idx != GaussT::nb_points; ++gauss_idx)
    {
      // This precomputes the required matrix operators
      u.support().compute_shape_functions(GaussT::instance().coords.col(gauss_idx));
      u.support().compute_jacobian(GaussT::instance().coords.col(gauss_idx));
      u.compute_values(GaussT::instance().coords.col(gauss_idx));

      const Real w = theta*GaussT::instance().weights[gauss_idx] * u.support().jacobian_determinant();

      adv = (w*tau_su) * (u.eval()*u.nabla()); // advection operator
      const Real b = -w*(u.shape_function()*delta_p)[0];
      for(Uint i = 0; i != dim; ++i)
      {
        const Real a = (u.nabla().row(i)*delta_p)[0];
        
        result.template segment<nb_nodes>(i*nb_nodes) += a*adv.transpose() + b*u.nabla().row(i).transpose();
      }
    }

    return result;
  }
};

static solver::actions::Proto::MakeSFOp<ApplyAup>::type const apply_aup = {};

template<typename ElementsT>
void NavierStokesSemiImplicit::set_elements_expressions( const std::string& name )
{ 
  static boost::proto::terminal< ElementSystemMatrix< boost::mpl::int_<2> > >::type const M = {};

  // Lumped mass matrix assembly
  m_mass_matrix_assembly->create_component<ProtoAction>(name)->set_expression(elements_expression(ElementsT(),
    group
    (
      _A = _0,
      element_quadrature( _A(u[_i], u[_i]) += transpose(N(u)) * N(u) ),
      lump(_A),
      m_u_lss->system_rhs += diagonal(_A)
    )
  ));
  

  // Assembly of the velocity matrices
  m_velocity_assembly->create_component<ProtoAction>(name)->set_expression(elements_expression(ElementsT(),
    group
    (
      _T(u,u) = _0, M(u,u) = _0,
      compute_tau.apply(u_adv, nu_eff, lit(dt), lit(tau_ps), lit(tau_su), lit(tau_bulk)),
      velocity_assembly(u, u_adv, nu_eff, M, _T, lit(dt), lit(tau_bulk)),
      m_u_lss->system_matrix += _T + lit(theta) * lit(dt) * M

//  _A = _0, _T = _0, M = _0,
//  compute_tau.apply(u, nu_eff, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
//  element_quadrature
//  (
//    M(u[_i], u[_i]) += nu_eff * transpose(nabla(u)) * nabla(u),
//    M(u[_i], u[_j]) += transpose(tau_bulk*nabla(u)[_i]) * nabla(u)[_j],
//    _T(u[_i], u[_i]) += transpose(N(u) + tau_su*u_adv*nabla(u)) * N(u)//,
//    //_A(u[_i], u[_i]) += transpose(N(u) + tau_su*u_adv*nabla(u)) * u_adv*nabla(u)
//    //_A(u[_i], u[_j]) += transpose(0.5*u_adv[_i]*(N(u) + tau_su*u_adv*nabla(u))) * nabla(u)[_j]
//  ),
//  m_u_lss->system_matrix += _T + lit(theta) * lit(dt) * (M + _A)
    )
  ));
  
  // Assembly of velocity RHS
  if(!options().value<bool>("enable_body_force"))
  {
    m_inner_loop->get_child("URHSAssembly")->create_component<ProtoAction>(name)->set_expression(elements_expression(ElementsT(),
    group
    (
      _A(u,u) = _0,
      compute_tau.apply(u_adv, nu_eff, lit(dt), lit(tau_ps), lit(tau_su), lit(tau_bulk)),
      m_u_lss->system_rhs += velocity_rhs(u_adv, nu_eff, lit(a), lit(dt)*(1. - lit(theta))*lit(a) - lit(u_vec), (1. - lit(theta))*lit(delta_p_sum) - lit(p_vec), lit(tau_su), lit(tau_bulk))
    )));
  }
  else
  {
    FieldVariable<7, VectorField> g("Force", "body_force");
    m_inner_loop->get_child("URHSAssembly")->create_component<ProtoAction>(name)->set_expression(elements_expression(ElementsT(),
    group
    (
      _A(u,u) = _0,
      compute_tau.apply(u_adv, nu_eff, lit(dt), lit(tau_ps), lit(tau_su), lit(tau_bulk)),
      m_u_lss->system_rhs += velocity_rhs(u_adv, nu_eff, g, lit(a), lit(dt)*(1. - lit(theta))*lit(a) - lit(u_vec), (1. - lit(theta))*lit(delta_p_sum) - lit(p_vec), lit(tau_su), lit(tau_bulk))
    )));
  }
  if(options().value<bool>("enable_boussinesq"))
  {
    FieldVariable<7, VectorField> g("Force", "body_force");
    FieldVariable<8, ScalarField> T("Temperature", "scalar_advection_solution");
    PhysicsConstant Tref("reference_temperature");
    m_inner_loop->get_child("URHSAssembly")->create_component<ProtoAction>(name+"_bousinesq")->set_expression(elements_expression(ElementsT(),
    group
    (
      _A(u,u) = _0, _a = _0,
      compute_tau.apply(u_adv, nu_eff, lit(dt), lit(tau_su)),
      element_quadrature
      (
         _a[u[_i]] += transpose(N(u) + tau_su*u_adv*nabla(u)) * g[_i] * (Tref - T)/T
      ),
      m_u_lss->system_rhs += _a
    )));
  }

  // Assembly of pressure RHS
  m_inner_loop->get_child("PRHSAssembly")->create_component<ProtoAction>(name)->set_expression(elements_expression(ElementsT(),
    group
    (
      _A(p,p) = _0, _a = _0,
      compute_tau.apply(u, nu_eff, lit(dt), lit(tau_ps), lit(tau_su), lit(tau_bulk)),
      m_p_lss->system_rhs += pressure_rhs(u_adv, lit(u_vec), lit(a), lit(delta_a), lit(p_vec), lit(tau_ps), lit(dt))
    )
  ));
  
  // Apply Aup to delta_p
  m_inner_loop->get_child("ApplyAup")->create_component<ProtoAction>(name)->set_expression(elements_expression(ElementsT(),
    group
    (
      _A(u,u) = _0, _a = _0,
      compute_tau.apply(u_adv, nu_eff, lit(dt), lit(tau_su)),
      m_u_lss->system_rhs += apply_aup(u_adv, lit(delta_p), lit(tau_su), lit(theta))
    )
  ));
}

} // UFEM
} // cf3

#endif // cf3_UFEM_semi_implicit_PressureMatrixAssembly_hpp
