// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_NavierStokesExplicitAssembly_hpp
#define cf3_UFEM_NavierStokesExplicitAssembly_hpp

#include "NavierStokesExplicit.hpp"

#include "solver/actions/Iterate.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

#include "SUPG.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;

using boost::proto::lit;


template<typename ElementsT>
void NavierStokesExplicit::set_velocity_assembly_expression(const std::string& base_name)
{
  m_inner_loop->add_component(create_proto_action
  (
    base_name+"VelocityAssembly",
    elements_expression
    (
      ElementsT(),
      group
      (
        _a = _0, _T = _0,
        compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
        element_quadrature
        (
          _a[u[_i]] += (nu_eff * transpose(nabla(u)) * nabla(u) // Diffusion
                       + transpose(N(u) + tau_su*u_adv*nabla(u)) * u_adv*nabla(u)) * transpose(transpose(nodal_values(u))[_i]) // Advection
                    -  (transpose(nabla(u)[_i])*N(p) - tau_su*transpose(u_adv*nabla(u)) * nabla(p)[_i]) / rho * nodal_values(p) // Pressure gradient (standard and SUPG)
                    +  transpose(N(u) + tau_su*u_adv*nabla(u)) * N(u) * transpose(transpose(nodal_values(a))[_i]), // Time, standard and SUPG
          _a[u[_i]] += transpose(0.5*u_adv[_i]*(N(u) + tau_su*u_adv*nabla(u)) + (tau_bulk + 0.33333333333333*nu_eff)*nabla(u)[_i]) * nabla(u)[_j] * transpose(transpose(nodal_values(u))[_j]), // Skew symmetry for advection and bulk viscosity
          _T(u[_i], u[_i]) += transpose(N(u)) * N(u)
        ),
        lump(_T),
        M += rho*diagonal(_T),
        R += -lit(rho)*_a
      )
  )));
}

struct PressureMatrix
{
  typedef void result_type;

  /// Compute the coefficients for the full Navier-Stokes equations
  template<typename UT, typename MT, typename MatrixT>
  void operator()(const UT& u, const MT& M, const Real& gamma_u, const Real& dt, const Real& tau_ps, MatrixT& A) const
  {
    typedef typename UT::EtypeT ElementT;

    static const Uint nb_nodes = ElementT::nb_nodes;
    static const Uint dim = ElementT::dimension;

    Eigen::Matrix<Real, dim*nb_nodes, nb_nodes> Aup;
    Eigen::Matrix<Real, nb_nodes, dim*nb_nodes> Apu;
    Eigen::Matrix<Real, nb_nodes, dim*nb_nodes> Tpu;

    typedef mesh::Integrators::GaussMappedCoords<2, ElementT::shape> GaussT;

    for(Uint gauss_idx = 0; gauss_idx != GaussT::nb_points; ++gauss_idx)
    {
      // This precomputes the required matrix operators
      u.support().compute_shape_functions(GaussT::instance().coords.col(gauss_idx));
      u.support().compute_coordinates();
      u.support().compute_jacobian(GaussT::instance().coords.col(gauss_idx));
      u.compute_values(GaussT::instance().coords.col(gauss_idx));

      for(Uint i = 0; i != dim; ++i)
      {
        Aup.template block<nb_nodes, nb_nodes>(i*nb_nodes, 0) = u.shape_function().transpose() * u.nabla().row(i);
        Apu.template block<nb_nodes, nb_nodes>(0, i*nb_nodes) = Aup.template block<nb_nodes, nb_nodes>(i*nb_nodes, 0).transpose()
          + tau_ps * (u.nabla().row(i).transpose() * u.eval()*u.nabla() + (u.eval()*u.nabla()*0.5).transpose() * u.nabla().row(i));
        Tpu.template block<nb_nodes, nb_nodes>(0, i*nb_nodes) = tau_ps * u.nabla().row(i).transpose() * u.shape_function();

        // Apply the inverse of the lumped mass matrix:
        for(Uint j = 0; j != nb_nodes; ++j)
        {
          Aup.template block<nb_nodes, nb_nodes>(i*nb_nodes, 0).col(j).array() /= M.value().col(i).array();
        }
      }

      A += GaussT::instance().weights[gauss_idx] * ( (Tpu + gamma_u*dt*Apu)*Aup );
    }
  }
};

static solver::actions::Proto::MakeSFOp<PressureMatrix>::type const pressure_matrix = {};

template<typename ElementsT>
void NavierStokesExplicit::set_pressure_assembly_expression(const std::string& base_name)
{
  m_pressure_lss->add_component(create_proto_action
  (
    base_name+"PressureAssembly",
    elements_expression
    (
      ElementsT(),
      group
      (
        _A(p,p) = _0, _a = _0, _T(p,p) = _0,
        compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
        pressure_matrix(u_adv, M, lit(gamma_u), lit(m_dt), lit(tau_ps), _A(p, p)),
        element_quadrature
        (
          _a[p]         += tau_ps * transpose(nabla(p)[_i]) * N(u) * transpose(transpose(nodal_values(a))[_i] + transpose(nodal_values(delta_a_star))[_i]) // PSPG, time part
                        +  transpose(N(p)) * nabla(u)[_i] * transpose(transpose(nodal_values(u))[_i] + lit(gamma_u)*lit(m_dt)*transpose(nodal_values(delta_a_star))[_i]) // G'u + gamma_u dt G'Delta_a*
                        +  tau_ps * (transpose(nabla(p)[_i]) * u_adv*nabla(u) + transpose(u_adv*nabla(p)*0.5) * nabla(u)[_i]) * transpose(transpose(nodal_values(u))[_i] + lit(gamma_u)*lit(m_dt)*transpose(nodal_values(delta_a_star))[_i]), // Standard + Skew symmetric advection PSPG term
          _T(p,p)       += tau_ps*transpose(nabla(p)) * nabla(p) / rho // Pressure PSPG
        ),
        m_pressure_lss->system_matrix += _A + _T,
        m_pressure_lss->system_rhs += -_a - _T * nodal_values(p)
      )
  )));
}

} // UFEM
} // cf3

#endif // cf3_UFEM_NavierStokesExplicitAssembly_hpp

