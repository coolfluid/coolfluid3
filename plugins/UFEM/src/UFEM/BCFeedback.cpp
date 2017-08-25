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

#include "math/LSS/System.hpp"

#include "mesh/Region.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Quad.hpp"
#include "mesh/LagrangeP0/Line.hpp"

#include "BCFeedback.hpp"
#include "AdjacentCellToFace.hpp"
#include "Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"
#include "solver/actions/Proto/SurfaceIntegration.hpp"

namespace cf3
{

namespace UFEM
{

using namespace solver::actions::Proto;
using boost::proto::lit;

common::ComponentBuilder < BCFeedback, common::Action, LibUFEM > BCFeedback_Builder;

BCFeedback::BCFeedback(const std::string& name) :
  Action(name),
  m_dirichlet(options().add("lss", Handle<math::LSS::System>()).pretty_name("LSS").description("The linear system for which the boundary condition is applied"))
{
  options().option("regions").add_tag("norecurse");
  options().add("field_tag", m_field_tag)
    .pretty_name("Field Tag")
    .description("Tag of the field to set a feedback on")
    .link_to(&m_field_tag)
    .mark_basic();

  options().add("variable_name", m_variable_name)
    .pretty_name("Variable Name")
    .description("Name of the variable to set the feedback on")
    .link_to(&m_variable_name)
    .mark_basic();

  options().add("factor", m_factor)
    .pretty_name("Factor")
    .description("Multiplication factor for the feedback")
    .link_to(&m_factor)
    .mark_basic();

  create_static_component<ProtoAction>("BC")->mark_basic().add_tag("norecurse");
}

BCFeedback::~BCFeedback()
{
}

void BCFeedback::execute()
{
  FieldVariable<0, ScalarField> phi(m_variable_name, m_field_tag);

  surface_integral(m_integral_result, m_loop_regions, phi * _norm(normal));
  m_integral_result /= compute_area(m_loop_regions);

  Handle<ProtoAction> bc(get_child("BC"));
  bc->set_expression(nodes_expression(group(m_dirichlet(phi) = lit(m_factor)*lit(m_integral_result))));
}

} // namespace UFEM

} // namespace cf3
