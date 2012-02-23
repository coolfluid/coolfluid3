// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the options facility"

#include <boost/assign/std/vector.hpp>
#include <boost/mpl/if.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/type_traits/is_base_of.hpp>

#include "common/BasicExceptions.hpp"
#include "common/Group.hpp"
#include "common/Core.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"

using namespace std;
using namespace boost::assign;

using namespace cf3;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( OptionsSuite )

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( StringOption )
{
  Component& root = Core::instance().root();

  root.options().add_option( "test_option", std::string("test01"));
  BOOST_CHECK_EQUAL(root.options().option("test_option").value_str(), "test01");

  root.options().option("test_option").change_value(std::string("test02"));
  BOOST_CHECK_EQUAL(root.options().option("test_option").value_str(), "test02");
}

BOOST_AUTO_TEST_CASE( ComponentOption )
{
  ExceptionManager::instance().ExceptionDumps = false;
  ExceptionManager::instance().ExceptionAborts = false;
  ExceptionManager::instance().ExceptionOutputs = false;

  Component& root = Core::instance().root();
  const Handle<Component> referred = root.create_component<Component>("ReferredComponent");

  OptionComponent<Component>& opt = root.options().add_option("test_component_option", root.handle<Component>());
  BOOST_CHECK(root.uri() == root.options().option("test_component_option").value< Handle<Component> >()->uri());
  BOOST_CHECK(root.name() == opt.value< Handle<Component> >()->name());

  root.options().option("test_component_option").change_value(referred);
  BOOST_CHECK(referred->uri() == root.options().option("test_component_option").value< Handle<Component> >()->uri());
  BOOST_CHECK(referred->name() == opt.value< Handle<Component> >()->name());

  const Handle<Group> group = root.create_component<Group>("TestGroup");
  OptionComponent<Group>& group_opt = root.options().add_option("test_group_option", Handle<Group>());
  BOOST_CHECK_THROW(root.options().option("test_group_option").change_value(referred), CastingFailed);

  root.options().option("test_group_option").change_value(group);
  BOOST_CHECK(group == group_opt.value< Handle<Group> >());
  
  Handle<Group const> const_group(group);
  BOOST_CHECK_THROW(root.options().option("test_group_option").change_value(const_group), CastingFailed);
  
  OptionComponent<Group const>& group_opt_const = root.options().add_option("test_const_group_option", Handle<Group const>());
  root.options().option("test_const_group_option").change_value(group);
  BOOST_CHECK(group == group_opt_const.value< Handle<Group const> >());
  
  root.options().option("test_const_group_option").change_value(Handle<Component>());
  BOOST_CHECK(is_null(group_opt_const.value< Handle<Group const> >()));
}

BOOST_AUTO_TEST_CASE( TestOptionArray )
{
  Component& root = Core::instance().root();

  std::vector<int> def;
  def += 1,2,3,4,5,6,7,8,9;
  BOOST_CHECK(root.options().add_option("test_array_option", def).value< std::vector<int> >() == def);

  BOOST_CHECK(def == root.options().option("test_array_option").value< std::vector<int> >());
}

BOOST_AUTO_TEST_CASE( TestOptionURI )
{
  Component& root = Core::instance().root();

  // Since the result is properly typed, we can immediately call supported_protocol
  root.options().add_option("test_uri_option", root.uri()).supported_protocol(cf3::common::URI::Scheme::CPATH);

  BOOST_CHECK(root.uri() == root.options().option("test_uri_option").value< URI >());
}


//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
