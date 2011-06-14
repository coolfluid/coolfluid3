// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/URI.hpp"
 
#include "Common/OptionArray.hpp"
#include "Common/OptionComponent.hpp"

#include "CActionDirector.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

/////////////////////////////////////////////////////////////////////////////////////

CActionDirector::CActionDirector(const std::string& name): CAction(name)
{
  m_properties.add_option< OptionArrayT <URI> >("ActionList", "Actions to execute in sequence")
    ->attach_trigger( boost::bind(&CActionDirector::trigger_actions, this) );
}

void CActionDirector::execute()
{
  const Uint nb_actions = m_actions.size();
  for(Uint i = 0; i != nb_actions; ++i)
    m_actions[i].lock()->execute();
}

CActionDirector& CActionDirector::append(const CAction& action)
{
  Property& actions_prop = property("ActionList");
  std::vector<URI> actions; actions_prop.put_value(actions);
  
  actions.push_back(action.uri());
  actions_prop.change_value(actions);
  
  return *this;
}

/////////////////////////////////////////////////////////////////////////////////////

void CActionDirector::trigger_actions()
{
  m_actions.clear();
  std::vector<URI> actions; property("ActionList").put_value(actions);
  m_actions.reserve(actions.size());
  BOOST_FOREACH(const URI& action_path, actions)
  {
    Component::Ptr action_component = access_component_ptr(action_path);
    if(!action_component)
      throw InvalidURI(FromHere(), "Action component with path " + action_path.path() + " does not exist");
    CAction::Ptr action = boost::dynamic_pointer_cast<CAction>(action_component);
    if(!action)
      throw InvalidURI(FromHere(), "Action component with path " + action_path.path() + " exists, but it is not a CAction");
    
    m_actions.push_back(boost::weak_ptr<CAction>(action));
  }
}


/////////////////////////////////////////////////////////////////////////////////////

CActionDirector& operator<<(CActionDirector& action_director, const CAction& action)
{
  return action_director.append(action);
}

/////////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

/////////////////////////////////////////////////////////////////////////////////////
