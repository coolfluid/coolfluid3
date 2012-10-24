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

#include "BCHoldValue.hpp"
#include "AdjacentCellToFace.hpp"
#include "Tags.hpp"

#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

namespace cf3
{

namespace UFEM
{

using namespace solver::actions::Proto;

common::ComponentBuilder < BCHoldValue, common::Action, LibUFEM > BCHoldValue_Builder;

BCHoldValue::BCHoldValue(const std::string& name) :
  ProtoAction(name),
  m_dirichlet(options().add("lss", Handle<math::LSS::System>()).pretty_name("LSS").description("The linear system for which the boundary condition is applied"))
{
  regist_signal( "set_tags" )
    .connect( boost::bind( &BCHoldValue::signal_set_tags, this, _1 ) )
    .description("Set the tags to use for the fields")
    .pretty_name("Set Tags")
    .signature( boost::bind(&BCHoldValue::signature_set_tags, this, _1) );
}

BCHoldValue::~BCHoldValue()
{
}

void BCHoldValue::set_tags ( const std::string& from_field_tag, const std::string& to_field_tag, const std::string& from_variable, const std::string& to_variable )
{
  FieldVariable<0, ScalarField> from(from_variable, from_field_tag);
  FieldVariable<1, ScalarField> to(to_variable, to_field_tag);
  
  set_expression(nodes_expression(group(m_dirichlet(to) = from, _cout << "  boundary temperature (from): " << from << "  boundary temperature (to): " << to <<  "\n")));
}

void BCHoldValue::signal_set_tags ( common::SignalArgs& node )
{
  common::XML::SignalOptions options(node);
  
  set_tags(options.option("from_field_tag").value<std::string>(),
           options.option("to_field_tag").value<std::string>(),
           options.option("from_variable").value<std::string>(),
           options.option("to_variable").value<std::string>());
}

void BCHoldValue::signature_set_tags ( common::SignalArgs& node )
{
  common::XML::SignalOptions options(node);
  
  options.add("from_field_tag", UFEM::Tags::solution())
    .pretty_name("From Field Tag")
    .description("Tag for the field to copy from");
    
  options.add("to_field_tag", UFEM::Tags::solution())
    .pretty_name("To Field Tag")
    .description("Tag for the field to copy to");
    
  options.add("from_variable", UFEM::Tags::solution())
    .pretty_name("From Variable")
    .description("Internal name for the variable to copy from");
    
  options.add("to_variable", UFEM::Tags::solution())
    .pretty_name("To Variable")
    .description("Internal name for the variable to copy to");
}

} // namespace UFEM

} // namespace cf3
