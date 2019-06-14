// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Core.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include <common/EventHandler.hpp>
#include <vector>
#include <iostream>
#include "math/LSS/System.hpp"
#include <cmath>
#include "mesh/Region.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "solver/actions/Proto/ElementGradDiv.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Line.hpp"
#include "solver/actions/Proto/Partial.hpp"

#include "BCAdjointpressurexNew.hpp"
#include "../AdjacentCellToFace.hpp"
#include "../Tags.hpp"

#include "solver/actions/Proto/Expression.hpp"

namespace cf3
{

namespace UFEM
{
namespace adjoint
{
using namespace solver::actions::Proto;
using boost::proto::lit;

common::ComponentBuilder < BCAdjointpressurexNew, common::Action, LibUFEMAdjoint > BCAdjointpressurexNew_Builder;

BCAdjointpressurexNew::BCAdjointpressurexNew(const std::string& name) :
  ProtoAction(name),
  system_rhs(options().add("lss", Handle<math::LSS::System>())
    .pretty_name("LSS")
    .description("The linear system for which the boundary condition is applied")),
  system_matrix(options().option("lss"))
  //m_dirichlet(options().add("lss", Handle<math::LSS::System>()).pretty_name("LSS").description("The linear system for which the boundary condition is applied"))
{

    options().add("turbulence", m_turbulence)
      .pretty_name("Adjoint of Adjoint ke")
      .description("Adjoint of Adjoint ke")
      .link_to(&m_turbulence) //0 if frozen turbulence
      .mark_basic(); // is this is enabled, the option can be accessed directly from Python, otherwise .options is needed

    FieldVariable<0, ScalarField> q("AdjPressure", "adjoint_solution");
    FieldVariable<1, VectorField> U("AdjVelocity", "adjoint_solution");
    FieldVariable<2, VectorField> u("Velocity", "navier_stokes_solution");
    FieldVariable<3, ScalarField> nu_eff("EffectiveViscosity", "navier_stokes_viscosity");
    FieldVariable<4, VectorField> grad_Ux("grad_Ux", "Adjvelocity_gradient");
    FieldVariable<5, ScalarField> epsilona("epsilona", "keAdjoint_solution");
    FieldVariable<6, ScalarField> ka("ka", "keAdjoint_solution");
    FieldVariable<7, ScalarField> epsilon("epsilon", "ke_solution");
    FieldVariable<8, ScalarField> k("k", "ke_solution");
    FieldVariable<9, VectorField> grad_ux("grad_ux", "velocity_gradient");
    //FieldVariable<10,VectorField> n("NodalNormal", "nodal_normals");


    // set_expression(nodes_expression(m_dirichlet(q) = (u[1]*U[1]) + (u[0]*U[0])+ (U[0]*n[0]+U[1]*n[1])*(u[0]*n[0]+u[1]*n[1]) + (nu_eff*grad_Ux[0]*n[0])+(nu_eff*grad_Ux[1]*n[1])-0.5*(u[0]*u[0]+u[1]*u[1])
    // -(u[0]*n[0]+u[1]*n[1])*(u[0]*n[0]+u[1]*n[1])));
    //-(u[0]*n[0]+u[1]*n[1])*(u[0]*n[0]+u[1]*n[1])-0.5*(u[0]*u[0]+u[1]*u[1])));
    //set_expression(nodes_expression(m_dirichlet(q)  = (transpose(u)*U)[0] + (transpose(u)*U)[1] + (u[0]*U[0]) + nu_eff*grad_Ux[0]*normal[0]+(nu_eff*grad_Ux[1])-(u[0]*u[0])-(u[1]*u[1])-0.5*(u[0]*u[0]+u[1]*u[1])));
    //
    // -(m_turbulence*2*((epsilona*m_c_epsilon_1*m_c_mu*k)+(ka*k*k*m_c_mu/epsilon))*grad_ux[0])));


    //set_expression(nodes_expression(m_dirichlet(q)  = (transpose(u)*U)[0] + (u[0]*U[0]) + (nu_eff*grad_Ux[0])));
    set_expression(elements_expression
    (
      boost::mpl::vector2<
          mesh::LagrangeP1::Triag3D,
          mesh::LagrangeP1::Line2D
          >(),
      group
      (
        _A(U) = _0, _A(q) = _0, _a[U] = _0, _a[q] = _0,
        element_quadrature
        (
          _A(U[_i],U[_j]) += transpose(N(U))*N(U)*(u[_j]*normal[_i]) / nu_eff, // Uj*uj*ni
          _A(U[_i],U[_i]) += transpose(N(U))*N(U)*((u*normal)[0]) / nu_eff, // Ui*uj*nj
          _A(U[_i], q) += -transpose(N(U))*N(q)*normal[_i] / nu_eff // -q*ni
        ),
        system_matrix+=_A,
        system_rhs += -_A*_x + _a
    )));



}

BCAdjointpressurexNew::~BCAdjointpressurexNew()
{
}


} // namespace adjointtube
} // namespace UFEM
} // namespace cf3
