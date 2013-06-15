// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for ActionDirector"

#include <iostream>

#include <boost/test/unit_test.hpp>

#include "common/CF.hpp"
#include "common/ActionDirector.hpp"
#include "common/Core.hpp"
#include "common/Foreach.hpp"
#include "common/URI.hpp"

using namespace cf3;
using namespace cf3::common;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( ActionDirectorSuite )

//////////////////////////////////////////////////////////////////////////////

/// Action that sets an integer, for testing purposes
struct SetIntegerAction : Action
{
  typedef boost::shared_ptr<SetIntegerAction> Ptr;
  typedef boost::shared_ptr<SetIntegerAction const> ConstPtr;
  SetIntegerAction(const std::string& name) : Action(name) {}
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
  Component& root = Core::instance().root();
  Handle<ActionDirector> director = root.create_component<ActionDirector>("director");
  Handle<SetIntegerAction> test_action = director->create_component<SetIntegerAction>("testaction");

  director->execute();
  BOOST_CHECK_EQUAL(test_action->value, 1);
}

BOOST_AUTO_TEST_CASE(ActionDirectorAppend)
{
  Component& root = Core::instance().root();
  
  Handle<ActionDirector> director(root.get_child("director"));
  Handle<SetIntegerAction> test_action2 = director->create_component<SetIntegerAction>("testaction2");
  
  BOOST_CHECK_EQUAL(test_action2->value, 1);
  director->execute();
  BOOST_CHECK_EQUAL(test_action2->value, 3);
}

BOOST_AUTO_TEST_CASE(ActionDirectorStream)
{
  Component& root = Core::instance().root();
  
  Handle<ActionDirector> director(root.get_child("director"));
  
  boost::shared_ptr<SetIntegerAction> test_action3_shared = allocate_component<SetIntegerAction>("testaction3");
  Handle<SetIntegerAction> test_action3_handle(test_action3_shared);
  SetIntegerAction& test_action3_ref = *test_action3_handle;
  
  // Overloaded shift-left operator for easy chaining of actions
  *director << test_action3_shared << test_action3_handle << test_action3_ref;
  
  BOOST_CHECK_EQUAL(test_action3_handle->value, 3);
  director->execute();
  BOOST_CHECK_EQUAL(test_action3_handle->value, 8);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

