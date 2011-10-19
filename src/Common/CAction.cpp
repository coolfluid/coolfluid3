// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Signal.hpp"

#include "Common/CAction.hpp"
#include "Common/FindComponents.hpp"

#include "Common/LibCommon.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////////////////

CAction::CAction ( const std::string& name ) : Component(name)
{
  // signals

  regist_signal( "execute" )
    ->connect( boost::bind( &CAction::signal_execute, this, _1 ) )
    ->description("Execute the action")
    ->pretty_name("Execute");
}


CAction& CAction::create_action(const std::string& action_provider,
                                      const std::string& name)
{
  CAction::Ptr sub_action = build_component_abstract_type<CAction>(action_provider,name);
  add_component(sub_action);
  return *sub_action;
}


void CAction::signal_execute ( common::SignalArgs& node )
{
  this->execute();
}

////////////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // cf3
