// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Signal.hpp"

#include "Common/CAction.hpp"
#include "Common/FindComponents.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/LibCommon.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

///////////////////////////////////////////////////////////////////////////////////////
  
CAction::CAction ( const std::string& name ) : Component(name)
{
  this->regist_signal ( "execute" , "Execute the action", "Execute" )->signal->connect ( boost::bind ( &CAction::signal_execute, this, _1 ) );
}

////////////////////////////////////////////////////////////////////////////////

CAction& CAction::create_action(const std::string& action_provider,
                                      const std::string& name)
{
  CAction::Ptr sub_action = create_component_abstract_type<CAction>(action_provider,name);
  add_component(sub_action);
  return *sub_action;
}

////////////////////////////////////////////////////////////////////////////////////

void CAction::signal_execute ( Common::SignalArgs& node )
{
  this->execute();
}

////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF
