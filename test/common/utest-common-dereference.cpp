// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::Component"

#include <type_traits>

#include <boost/test/unit_test.hpp>

#include "common/Group.hpp"
#include "common/DereferenceComponent.hpp"

using namespace cf3;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( Component_TestSuite )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Dereference )
{
  // constructor with passed path
  boost::shared_ptr<Component> root_shared = boost::static_pointer_cast<Component>(allocate_component<Group>("root"));
  boost::shared_ptr<Component const> root_shared_const = root_shared;
  Component& ref = *root_shared;
  const Component& ref_const = *root_shared;
  Component* ptr = root_shared.get();
  const Component* ptr_const = root_shared.get();
  Handle<Component> handle = root_shared->handle();
  Handle<Component const> handle_const = root_shared->handle();

  BOOST_CHECK(common::dereference(root_shared).name() == "root");
  BOOST_CHECK(common::dereference(root_shared_const).name() == "root");
  BOOST_CHECK(common::dereference(ref).name() == "root");
  BOOST_CHECK(common::dereference(ref_const).name() == "root");
  BOOST_CHECK(common::dereference(ptr).name() == "root");
  BOOST_CHECK(common::dereference(ptr_const).name() == "root");
  BOOST_CHECK(common::dereference(handle).name() == "root");
  BOOST_CHECK(common::dereference(handle_const).name() == "root");

  static_assert(std::is_const<std::remove_reference<decltype(common::dereference(handle_const))>::type>::value, "expected const");
  static_assert(std::is_const<std::remove_reference<decltype(common::dereference(ref_const))>::type>::value, "expected const");
  static_assert(std::is_const<std::remove_reference<decltype(common::dereference(ptr_const))>::type>::value, "expected const");
  static_assert(std::is_const<std::remove_reference<decltype(common::dereference(root_shared_const))>::type>::value, "expected const");

}

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
