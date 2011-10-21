// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CActionDirector"

#include <iostream>

#include <boost/test/unit_test.hpp>

#include "common/CF.hpp"
#include "common/CActionDirector.hpp"
#include "common/Core.hpp"
#include "common/CRoot.hpp"
#include "common/Foreach.hpp"
#include "common/URI.hpp"

using namespace cf3;
using namespace cf3::common;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( ActionDirectorSuite )

//////////////////////////////////////////////////////////////////////////////

/// Action that sets an integer, for testing purposes
struct SetIntegerAction : CAction
{
  typedef boost::shared_ptr<SetIntegerAction> Ptr;
  typedef boost::shared_ptr<SetIntegerAction const> ConstPtr;
  SetIntegerAction(const std::string& name) : CAction(name) {}
  static std::string type_name () { return "SetIntegerAction"; }
  virtual void execute()
  {
    ++value;
  }

  static Uint value;
};

Uint SetIntegerAction::value = 0;

BOOST_AUTO_TEST_CASE(ActionDirectorBasic)
{
  CRoot& root = Core::instance().root();
  CActionDirector::Ptr director = root.create_component_ptr<CActionDirector>("director");
  SetIntegerAction::Ptr test_action = director->create_component_ptr<SetIntegerAction>("testaction");
  const std::vector<std::string> action_vector(1, test_action->name());
  director->configure_option("action_order", action_vector);
  BOOST_CHECK_EQUAL(test_action->value, 0);
  director->execute();
  BOOST_CHECK_EQUAL(test_action->value, 1);
}

BOOST_AUTO_TEST_CASE(ActionDirectorAppend)
{
  CRoot& root = Core::instance().root();
  
  CActionDirector& director = dynamic_cast<CActionDirector&>(root.get_child("director"));
  SetIntegerAction& test_action2 = director.create_component<SetIntegerAction>("testaction2");
  director.append(test_action2);
  
  std::vector<std::string> actions; director.option("action_order").put_value(actions);
  BOOST_CHECK_EQUAL(actions.size(), 2);
  
  BOOST_CHECK_EQUAL(test_action2.value, 1);
  director.execute();
  BOOST_CHECK_EQUAL(test_action2.value, 3);
}

BOOST_AUTO_TEST_CASE(ActionDirectorStream)
{
  CRoot& root = Core::instance().root();
  
  CActionDirector& director = dynamic_cast<CActionDirector&>(root.get_child("director"));
  SetIntegerAction& test_action3 = director.create_component<SetIntegerAction>("testaction3");
  
  // Overloaded shift-left operator for easy chaining of actions
  director << test_action3 << test_action3 << test_action3;
  
  std::vector<std::string> actions; director.option("action_order").put_value(actions);
  BOOST_CHECK_EQUAL(actions.size(), 5);
  
  BOOST_CHECK_EQUAL(test_action3.value, 3);
  director.execute();
  BOOST_CHECK_EQUAL(test_action3.value, 8);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

