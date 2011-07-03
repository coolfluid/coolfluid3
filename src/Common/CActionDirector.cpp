// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/URI.hpp"

#include "Common/CBuilder.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionComponent.hpp"

#include "CActionDirector.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

ComponentBuilder < CActionDirector, CAction, LibCommon > CActionDirector_Builder;
  
/////////////////////////////////////////////////////////////////////////////////////

CActionDirector::CActionDirector(const std::string& name): CAction(name)
{
  m_options.add_option< OptionArrayT <std::string> >("ActionList", "Names of the actions to execute in sequence");
}

void CActionDirector::execute()
{
  Option& actions_prop = option("ActionList");
  std::vector<std::string> actions; actions_prop.put_value(actions);
  
  BOOST_FOREACH(const std::string& action_name, actions)
  {
    dynamic_cast<CAction&>(get_child(action_name)).execute();
  }
}

CActionDirector& CActionDirector::append(const CAction& action)
{
  Option& actions_prop = option("ActionList");
  std::vector<std::string> actions; actions_prop.put_value(actions);

  actions.push_back(action.name());
  actions_prop.change_value(actions);

  return *this;
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
