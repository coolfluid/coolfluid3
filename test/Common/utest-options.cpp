// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the options facility"

#include <iostream>

#include <boost/test/unit_test.hpp>

#include "Common/BasicExceptions.hpp"
#include "Common/CGroup.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionT.hpp"
#include "Common/PropertyList.hpp"

using namespace std;
using namespace boost;

using namespace CF;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( OptionsSuite )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( StringOption )
{
  CRoot& root = Core::instance().root();
  
  root.properties().add_option< OptionT<std::string> >("test_option", "Test Option", "test01");
  BOOST_CHECK_EQUAL(root.property("test_option").value_str(), "test01");
  
  root.property("test_option").as_option().change_value(std::string("test02"));
  BOOST_CHECK_EQUAL(root.property("test_option").value_str(), "test02");
}

BOOST_AUTO_TEST_CASE( ComponentOption )
{
  ExceptionManager::instance().ExceptionDumps = false;
  ExceptionManager::instance().ExceptionAborts = false;
  ExceptionManager::instance().ExceptionOutputs = false;
  
  CRoot& root = Core::instance().root();
  const Component& referred = root.create_component<Component>("ReferredComponent");
  
  OptionComponent<Component>::Ptr opt = boost::dynamic_pointer_cast< OptionComponent<Component> >(root.properties().add_option< OptionComponent<Component> >("test_component_option", "Test component option", root.uri()));
  BOOST_CHECK(root.uri() == root.property("test_component_option").as_option().value<URI>());
  BOOST_CHECK(root.name() == opt->component().name());
  
  root.property("test_component_option").as_option().change_value(referred.uri());
  BOOST_CHECK(referred.uri() == root.property("test_component_option").as_option().value<URI>());
  BOOST_CHECK(referred.name() == opt->component().name());
  
  const CGroup& group = root.create_component<CGroup>("TestGroup");
  OptionComponent<CGroup>::Ptr group_opt = boost::dynamic_pointer_cast< OptionComponent<CGroup> >(root.properties().add_option< OptionComponent<CGroup> >("test_group_option", "Test group option", URI()));
  try
  {
    root.property("test_group_option").as_option().change_value(referred.uri());
    BOOST_CHECK(false); // Above should throw
  }
  catch(CastingFailed&)
  {
    BOOST_CHECK(true);
  }
  
  root.property("test_group_option").as_option().change_value(group.uri());
  BOOST_CHECK(&group == &group_opt->component());
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
