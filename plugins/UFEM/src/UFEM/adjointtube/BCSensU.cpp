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

#include "BCSensU.hpp"
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

common::ComponentBuilder < BCSensU, common::Action, LibUFEMAdjointTube > BCSensU_Builder;

BCSensU::BCSensU(const std::string& name) :
  ProtoAction(name),
  m_dirichlet(options().add("lss", Handle<math::LSS::System>()).pretty_name("LSS").description("The linear system for which the boundary condition is applied"))
{

    options().add("turbulence", m_turbulence)
      .pretty_name("Adjoint of Adjoint ke")
      .description("Adjoint of Adjoint ke")
      .link_to(&m_turbulence) //0 if frozen turbulence
      .mark_basic(); // is this is enabled, the option can be accessed directly from Python, otherwise .options is needed


  FieldVariable<0, VectorField> grad_ux("grad_ux", "velocity_gradient");
  FieldVariable<1, VectorField> SensU("SensU", "sensitivity_solution");
  FieldVariable<2, VectorField> grad_uy("grad_uy", "velocity_gradient");
  FieldVariable<3, VectorField> n("NodalNormal", "nodal_normals");


  // set_expression(nodes_expression(m_dirichlet(SensU)  = (transpose(grad_ux))));
  // Deze randvoorwaarde moet elementsgewijs gedefinieerd worden.

  set_expression(nodes_expression(m_dirichlet(SensU[0])  = (grad_ux[0])));//*n[0]+grad_ux[1]*n[1])));
  //set_expression(nodes_expression(m_dirichlet(SensU[1])  = (grad_uy[0])));//*n[0]+grad_uy[1]*n[1])));

  // detail::set_result(SensU, n, grad_ux,grad_uy)






}

BCSensU::~BCSensU()
{
}


} // namespace adjointtube
} // namespace UFEM
} // namespace cf3
