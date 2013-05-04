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

#include "UFEM/SUPG.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

using boost::proto::lit;

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
    Eigen::Matrix<Real, nb_nodes, nb_nodes> N_nabla_i;
    Eigen::Matrix<Real, nb_nodes, nb_nodes> adv_nabla_i;
    Eigen::Matrix<Real, nb_nodes, dim*nb_nodes> Apu;
    Eigen::Matrix<Real, nb_nodes, dim*nb_nodes> Tpu;
    Eigen::Matrix<Real, nb_nodes, nb_nodes> App;

    App.setZero();
    Apu.setZero();
    Tpu.setZero();

    typedef mesh::Integrators::GaussMappedCoords<2, ElementT::shape> GaussT;

    for(Uint gauss_idx = 0; gauss_idx != GaussT::nb_points; ++gauss_idx)
    {
      // This precomputes the required matrix operators
      u.support().compute_shape_functions(GaussT::instance().coords.col(gauss_idx));
      u.support().compute_jacobian(GaussT::instance().coords.col(gauss_idx));
      u.compute_values(GaussT::instance().coords.col(gauss_idx));

      const Real w = GaussT::instance().weights[gauss_idx] * u.support().jacobian_determinant();
      const Real tau_w = w*tau_ps;

      adv = tau_w*u.eval() * u.nabla(); // advection operator
      App += tau_w * u.nabla().transpose() * u.nabla();
      for(Uint i = 0; i != dim; ++i)
      {
        N_nabla_i = w*u.shape_function().transpose() * u.nabla().row(i);
        adv_nabla_i = adv.transpose() * u.nabla().row(i);
        Apu.template block<nb_nodes, nb_nodes>(0, i*nb_nodes) += N_nabla_i + 0.5*adv_nabla_i + adv_nabla_i.transpose();
        Tpu.template block<nb_nodes, nb_nodes>(0, i*nb_nodes) += tau_ps*N_nabla_i.transpose();
      }
    }

    result = App*p_vec;
    for(Uint i = 0; i != dim; ++i)
    {
      result += Apu.template block<nb_nodes, nb_nodes>(0, i*nb_nodes) * (u_vec.row(i) + dt*delta_a.row(i)).transpose()
        + Tpu.template block<nb_nodes, nb_nodes>(0, i*nb_nodes) * (a.row(i) + delta_a.row(i)).transpose();
    }

    return result;
  }
};

static solver::actions::Proto::MakeSFOp<PressureRHS>::type const pressure_rhs = {};

// struct VelocityRHS
// {
//   template<typename Signature>
//   struct result;
// 
//   template<typename This, typename UT, typename UVecT, typename PVecT>
//   struct result<This(UT, UVecT, UVecT, UVecT, PVecT, Real, Real)>
//   {
//     typedef const EIgen::Matrix<Real, UT::EtypeT::nb_nodes*UT::EtypeT::dimension, 1>& type;
//   };
// 
//   /// Compute the coefficients for the full Navier-Stokes equations
//   template<typename StorageT, typename UT, typename UVecT, typename PVecT>
//   const StorageT& operator()(StorageT& result, const UT& u, const UVecT& a, const PVecT& p_vec, const PVecT& delta_p_sum, const Real& tau_su, const Real& theta) const
//   {
//     typedef typename UT::EtypeT ElementT;
// 
//     static const Uint nb_nodes = ElementT::nb_nodes;
//     static const Uint dim = ElementT::dimension;
// 
//     Eigen::Matrix<Real, 1, nb_nodes> adv;
//     Eigen::Matrix<Real, 1, nb_nodes> wN; // weighted shape function
//     Eigen::Matrix<Real, dim*nb_nodes, dim*nb_nodes> Aup;
//     Eigen::Matrix<Real, nb_nodes, nb_nodes> Tuu;
// 
//     Aup.setZero();
//     Tuu.setZero();
// 
//     typedef mesh::Integrators::GaussMappedCoords<2, ElementT::shape> GaussT;
// 
//     for(Uint gauss_idx = 0; gauss_idx != GaussT::nb_points; ++gauss_idx)
//     {
//       // This precomputes the required matrix operators
//       u.support().compute_shape_functions(GaussT::instance().coords.col(gauss_idx));
//       u.support().compute_jacobian(GaussT::instance().coords.col(gauss_idx));
//       u.compute_values(GaussT::instance().coords.col(gauss_idx));
// 
//       const Real w = GaussT::instance().weights[gauss_idx] * u.support().jacobian_determinant();
//       const Real tau_w = w*tau_su;
// 
//       adv = tau_w*u.eval() * u.nabla(); // advection operator
//       wN = w*u.shape_function();
//       for(Uint i = 0; i != dim; ++i)
//       {
//         Aup.template block<nb_nodes, nb_nodes>(i*nb_nodes, 0) += adv.transpose()*u.nabla().row(i) - u.nabla.row(i).transpose()*wN;
//         Tpu.template block<nb_nodes, nb_nodes>(0, i*nb_nodes) += tau_ps*N_nabla_i.transpose();
//       }
//     }
// 
//     result = App*p_vec;
//     for(Uint i = 0; i != dim; ++i)
//     {
//       result += Apu.template block<nb_nodes, nb_nodes>(0, i*nb_nodes) * (u_vec.row(i) + dt*delta_a.row(i)).transpose()
//         + Tpu.template block<nb_nodes, nb_nodes>(0, i*nb_nodes) * (a.row(i) + delta_a.row(i)).transpose();
//     }
// 
//     return result;
//   }
// };
// 
// static solver::actions::Proto::MakeSFOp<VelocityRHS>::type const velocity_rhs = {};

template<typename ElementsT>
void NavierStokesSemiImplicit::set_elements_expressions( const std::string& name )
{ 
  static boost::proto::terminal< ElementSystemMatrix< boost::mpl::int_<2> > >::type const M = {};
  
  // Pressure system assembly
  m_pressure_assembly->create_component<ProtoAction>(name)->set_expression(elements_expression(ElementsT(),
    group
    (
      _A = _0,
      compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
      element_quadrature( _A(p,p) += transpose(nabla(p)) * nabla(p) ),
      m_p_lss->system_matrix += -lit(theta) * (lit(tau_ps) + lit(dt)) * _A
    )
  ));
  
  // Lumped mass matrix assembly
  m_mass_matrix_assembly->create_component<ProtoAction>(name)->set_expression(elements_expression(ElementsT(),
    group
    (
      _A = _0,
      element_quadrature( _A(u[_i], u[_i]) += transpose(N(u)) * N(u) ),
      lump(_A),
      m_auu_lss->system_rhs += diagonal(_A)
    )
  ));
  
  // Assembly of the velocity matrices
  m_velocity_assembly->create_component<ProtoAction>(name)->set_expression(elements_expression(ElementsT(),
    group
    (
      _A = _0, _T = _0, M = _0,
      compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
      element_quadrature
      (
        M(u[_i], u[_i]) += nu_eff * transpose(nabla(u)) * nabla(u),
        M(u[_i], u[_j]) += transpose((tau_bulk + 0.33333333333333*nu_eff)*nabla(u)[_i]) * nabla(u)[_j],
        _T(u[_i], u[_i]) += transpose(N(u) + tau_su*u_adv*nabla(u)) * N(u),
        _A(u[_i], u[_i]) += transpose(N(u) + tau_su*u_adv*nabla(u)) * u_adv*nabla(u),
        _A(u[_i], u[_j]) += transpose(0.5*u_adv[_i]*(N(u) + tau_su*u_adv*nabla(u))) * nabla(u)[_j]
        
      ),
      m_u_lss->system_matrix += _T + lit(theta) * lit(dt) * M,
      m_auu_lss->system_matrix += _A + M
    )
  ));
  
  // Assembly of velocity RHS
  m_inner_loop->get_child("URHSAssembly")->create_component<ProtoAction>(name)->set_expression(elements_expression(ElementsT(),
    group
    (
      _A(u,u) = _0, _a = _0,
      compute_tau(u, nu_eff, lit(tau_su)),
      element_quadrature
      (
        _a[u[_i]] += (transpose(tau_su*u_adv*nabla(u)) * nabla(u)[_i] - transpose(nabla(u)[_i]) * N(u)) * ((1. - lit(theta))*delta_p_sum - p_vec) // Aup
                   - (transpose(N(u) + tau_su*u_adv*nabla(u)) * N(u)) * transpose(lit(a)[_i]) // Tuu
      ),
      m_u_lss->system_rhs += _a
    )
  ));
  
  // Assembly of pressure RHS
  m_inner_loop->get_child("PRHSAssembly")->create_component<ProtoAction>(name)->set_expression(elements_expression(ElementsT(),
    group
    (
      _A(p,p) = _0, _a = _0,
      compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
//       element_quadrature
//       (
//         _a[p] += (transpose(N(p) + tau_ps*u_adv*nabla(p)*0.5) * nabla(p)[_i] + tau_ps * transpose(nabla(p)[_i]) * u_adv*nabla(p)) * transpose(lit(u_vec)[_i] + lit(dt)*lit(delta_a)[_i]) // Apu
//               +   tau_ps * transpose(nabla(p)[_i]) * N(p) * transpose(lit(a)[_i] + lit(delta_a)[_i]), // Tpu
//         _A(p,p) += tau_ps * transpose(nabla(p)) * nabla(p)
//       ),
//       m_p_lss->system_rhs += _a + _A*p_vec
      m_p_lss->system_rhs += pressure_rhs(u_adv, lit(u_vec), lit(a), lit(delta_a), lit(p_vec), lit(tau_ps), lit(dt))
    )
  ));
  
  // Apply Aup to delta_p
  m_inner_loop->get_child("ApplyAup")->create_component<ProtoAction>(name)->set_expression(elements_expression(ElementsT(),
    group
    (
      _A(u,u) = _0, _a = _0,
      compute_tau(u, nu_eff, lit(tau_su)),
      element_quadrature
      (
        _a[u[_i]] += (transpose(tau_su*u_adv*nabla(u)) * nabla(u)[_i] - transpose(nabla(u)[_i]) * N(u)) * delta_p // Aup
      ),
      m_u_lss->system_rhs += lit(theta) * _a
    )
  ));
}

} // UFEM
} // cf3

#endif // cf3_UFEM_semi_implicit_PressureMatrixAssembly_hpp
