// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::common::handle"

#include <boost/test/unit_test.hpp>

#include "common/Link.hpp"
#include "common/Group.hpp"
#include "common/Handle.hpp"

using namespace cf3;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(HandleSuite)

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( NullHandle )
{
  Handle<Group> a, b;

  BOOST_CHECK(!a);
  BOOST_CHECK(a == b);
  BOOST_CHECK(is_null(a));
  BOOST_CHECK(a == nullptr);
  BOOST_CHECK(nullptr == a);
  BOOST_CHECK(!is_not_null(a));
  BOOST_CHECK(!(nullptr != a));
  BOOST_CHECK(!(a != nullptr));
}

BOOST_AUTO_TEST_CASE( ValidHandle )
{
  boost::shared_ptr<Group> g = allocate_component<Group>("group");
  Handle<Group> a(g);

  BOOST_CHECK(is_not_null(a));
  BOOST_CHECK_EQUAL(a->name(), "group");
  BOOST_CHECK_EQUAL((*a).name(), "group");
  BOOST_CHECK_EQUAL(a.get()->name(), "group");

  g.reset();
  BOOST_CHECK(is_null(a));
  BOOST_CHECK(is_null(a.get()));
}

BOOST_AUTO_TEST_CASE( Conversion )
{
  boost::shared_ptr<Group> group = allocate_component<Group>("group");
  Handle<Group> group_handle(group);

  Handle<Component> a(group);
  BOOST_CHECK(is_not_null(a));
  BOOST_CHECK_EQUAL(a->name(), "group");

  Handle<Component> b(group_handle);
  BOOST_CHECK(is_not_null(b));
  BOOST_CHECK_EQUAL(b->name(), "group");

  // Successful dynamic cast
  Handle<Group> c(b);
  BOOST_CHECK(is_not_null(c));
  BOOST_CHECK_EQUAL(c->name(), "group");

  // Bad dynamic cast
  Handle<Link> d(b);
  BOOST_CHECK(is_null(d));

  // Bad dynamic cast
  Handle<Link> e(c);
  BOOST_CHECK(is_null(e));

  // Successful dynamic cast
  Handle<Group> f = b->handle<Group>();
  BOOST_CHECK(is_not_null(f));
  BOOST_CHECK_EQUAL(f->name(), "group");

  // Bad dynamic cast
  Handle<Link> g = b->handle<Link>();
  BOOST_CHECK(is_null(g));

  // Construction from const
  boost::shared_ptr<Group const> const_shared(group);
  Handle<Group const> const_handle(const_shared);
  BOOST_CHECK_EQUAL(const_handle->name(), "group");
  //Handle<Group> bad(const_shared); // these shouldn't compile
  //Handle<Group> bad2(const_handle);

  // Check releasing
  group.reset();
  const_shared.reset();
  BOOST_CHECK(is_null(a));
  BOOST_CHECK(is_null(b));
  BOOST_CHECK(is_null(c));
  BOOST_CHECK(is_null(d));
  BOOST_CHECK(is_null(e));

}

BOOST_AUTO_TEST_CASE( Reassign )
{
  boost::shared_ptr<Group> group = allocate_component<Group>("group");

  Handle<Group> group_handle(group);
  BOOST_CHECK_EQUAL("group", group_handle->name());

  group = allocate_component<Group>("group2");
  BOOST_CHECK(!group_handle); // should have expired
  group_handle = Handle<Group>(group);
  BOOST_CHECK_EQUAL("group2", group_handle->name());
}

BOOST_AUTO_TEST_CASE( AssignToConst )
{
  boost::shared_ptr<Group> group = allocate_component<Group>("group");

  Handle<Group> group_handle(group);
  Handle<Group const> const_group_handle = group_handle;

  BOOST_CHECK_EQUAL("group", const_group_handle->name());
}

BOOST_AUTO_TEST_CASE( AssignToBase )
{
  boost::shared_ptr<Group> group = allocate_component<Group>("group");

  Handle<Group> group_handle(group);
  Handle<Component> comp_handle = group_handle;
  Handle<Component const> const_comp_handle = group_handle;

  //Handle<Group> try_dynamic_cast = comp_handle; // Won't compile (and shouldn't!)
  //Handle<Component> try_const_cast = const_comp_handle; // Won't compile (and shouldn't!)

  BOOST_CHECK_EQUAL("group", comp_handle->name());
  BOOST_CHECK_EQUAL("group", const_comp_handle->name());
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
