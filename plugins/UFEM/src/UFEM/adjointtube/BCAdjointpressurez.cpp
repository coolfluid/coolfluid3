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

#include "BCAdjointpressurez.hpp"
#include "../AdjacentCellToFace.hpp"
#include "../Tags.hpp"

#include "solver/actions/Proto/Expression.hpp"

namespace cf3
{

namespace UFEM
{
namespace adjointtube
{
using namespace solver::actions::Proto;
using boost::proto::lit;

common::ComponentBuilder < BCAdjointpressurez, common::Action, LibUFEMAdjointTube > BCAdjointpressurez_Builder;

BCAdjointpressurez::BCAdjointpressurez(const std::string& name) :
  ProtoAction(name),
  m_dirichlet(options().add("lss", Handle<math::LSS::System>()).pretty_name("LSS").description("The linear system for which the boundary condition is applied"))
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
    FieldVariable<4, VectorField> grad_Uz("grad_Uz", "Adjvelocity_gradient");
    FieldVariable<5, ScalarField> epsilona("epsilona", "keAdjoint_solution");
    FieldVariable<6, ScalarField> ka("ka", "keAdjoint_solution");
    FieldVariable<7, ScalarField> epsilon("epsilon", "ke_solution");
    FieldVariable<8, ScalarField> k("k", "ke_solution");
    FieldVariable<9, VectorField> grad_uz("grad_uz","velocity_gradient");

    set_expression(nodes_expression(m_dirichlet(q)  = (transpose(u)*U)[0] + (u[2]*U[2]) + (nu_eff*grad_Uz[2])-(m_turbulence*2*((epsilona*m_c_epsilon_1*m_c_mu*k)+(ka*k*k*m_c_mu/epsilon))*grad_uz[2])));



}

BCAdjointpressurez::~BCAdjointpressurez()
{
}


} // namespace adjointtube
} // namespace UFEM
} // namespace cf3
