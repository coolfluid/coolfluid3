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
                    +  transpose(N(u) + tau_su*u*nabla(u)) * u*nabla(u)) * transpose(transpose(nodal_values(u))[_i]) // Advection
                    -  (transpose(nabla(u)[_i])*N(p) - tau_su*transpose(u*nabla(u)) * nabla(p)[_i]) / rho * nodal_values(p) // Pressure gradient (standard and SUPG)
                    +  transpose(N(u) + tau_su*u*nabla(u)) * N(u) * transpose(transpose(nodal_values(a))[_i]), // Time, standard and SUPG
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
  void operator()(const UT& u, const MT& M, const Real& rho, const Real& tau_ps, const Real& tau_su, const Real& gamma_u, const Real& dt, MatrixT& A) const
  {
    typedef typename UT::EtypeT ElementT;

    static const Uint nb_nodes = ElementT::nb_nodes;
    static const Uint dim = ElementT::dimension;

    Eigen::Matrix<Real, dim*nb_nodes, nb_nodes> Aup;
    Eigen::Matrix<Real, nb_nodes, dim*nb_nodes> Apu;

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
        Apu.template block<nb_nodes, nb_nodes>(0, i*nb_nodes) = Aup.template block<nb_nodes, nb_nodes>(i*nb_nodes, 0).transpose();

        // Apply the inverse of the lumped mass matrix:
        for(Uint j = 0; j != nb_nodes; ++j)
        {
          Aup.template block<nb_nodes, nb_nodes>(i*nb_nodes, 0).col(j).array() /= M.value().col(i).array();
        }
      }

      A += GaussT::instance().weights[gauss_idx] * ( gamma_u*dt*Apu*Aup );
    }
  }
};

static solver::actions::Proto::MakeSFOp<PressureMatrix>::type const pressure_matrix = {};

template<typename ElementsT>
void NavierStokesExplicit::set_pressure_assembly_expression(const std::string& base_name)
{
  m_inner_loop->add_component(create_proto_action
  (
    base_name+"PressureAssembly",
    elements_expression
    (
      ElementsT(),
      group
      (
        _A(p,p) = _0, _a = _0,
        compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
        pressure_matrix(u, M, rho, lit(tau_ps), lit(tau_su), lit(gamma_u), lit(dt()), _A(p, p)),
        element_quadrature
        (
          _a[p]         += tau_ps * transpose(nabla(p)[_i]) * N(u) * transpose(transpose(nodal_values(a))[_i]) // PSPG, time part
                        +  transpose(N(p)) * nabla(u)[_i] * transpose(transpose(nodal_values(u))[_i] + lit(gamma_u)*lit(dt())*transpose(nodal_values(delta_a_star))[_i]), // G'u + gamma_u dt G'Delta_a*
          _a[p]         += tau_ps*transpose(nabla(p)) * nabla(p) / rho * nodal_values(p) // Pressure PSPG
        ),
        system_matrix += _A,
        system_rhs += -_a
      )
  )));
}

} // UFEM
} // cf3

#endif // cf3_UFEM_NavierStokesExplicitAssembly_hpp

