// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for iteration over components"

#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/Component.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/CRoot.hpp"
#include "Common/CGroup.hpp"
#include "Common/CLink.hpp"

using namespace CF;
using namespace CF::Common;

//////////////////////////////////////////////////////////////////////////////

struct ComponentIterationFixture
{
  /// common setup for each test case
  ComponentIterationFixture()
  {
    m_root = CRoot::create ( "root" );
    Component::Ptr comp1 = m_root->create_component_type<Component>("comp1");
    top_component_names.push_back(comp1->name());
    component_names.push_back(comp1->name());
    Component::Ptr comp1_1 = comp1->create_component_type<Component>("comp1_1");
    component_names.push_back(comp1_1->name());
    Component::Ptr comp1_2 = comp1->create_component_type<Component>("comp1_2");
    component_names.push_back(comp1_2->name());
    Component::Ptr comp2   = m_root->create_component_type<Component>("comp2");
    top_component_names.push_back(comp2->name());
    component_names.push_back(comp2->name());
    Component::Ptr comp2_1 = comp2->create_component_type<Component>("comp2_1");
    component_names.push_back(comp2_1->name());
    Component::Ptr comp2_2 = comp2->create_component_type<Component>("comp2_2");
    component_names.push_back(comp2_2->name());
    CGroup::Ptr group1 = m_root->create_component_type<CGroup>("group1");
    top_component_names.push_back(group1->name());
    component_names.push_back(group1->name());
    top_group_names.push_back(group1->name());
    group_names.push_back(group1->name());
    CGroup::Ptr group1_1 = group1->create_component_type<CGroup>("group1_1");
    component_names.push_back(group1_1->name());
    group_names.push_back(group1_1->name());
    CGroup::Ptr group1_2 = group1->create_component_type<CGroup>("group1_2");
    component_names.push_back(group1_2->name());
    group_names.push_back(group1_2->name());
    CGroup::Ptr group2 = m_root->create_component_type<CGroup>("group2");
    top_component_names.push_back(group2->name());
    component_names.push_back(group2->name());
    top_group_names.push_back(group2->name());
    group_names.push_back(group2->name());
  }

  /// common tear-down for each test case
  ~ComponentIterationFixture()
  {
  }

  /// mutable root
  Component& root() { return *m_root; }

  /// const root
  const Component& const_root() { return *m_root; }

  /// list of all component names on the first level
  std::vector<std::string> top_component_names;
  /// list of all component names
  std::vector<std::string> component_names;
  /// list of all group names at the top level
  std::vector<std::string> top_group_names;
  /// list of all group names
  std::vector<std::string> group_names;

private:
  Component::Ptr m_root;
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( ComponentIteration, ComponentIterationFixture )

//////////////////////////////////////////////////////////////////////////////

// Iterator tests:
// ---------------
// Several ways are available to iterate using a filter or predicate.
// They are listed from most difficult to use (showing how to do it yourself -> BOILER-PLATE-CODE)
// to most easy to use (wrappers that make users life easy)


//////////////////////////////////////////////////////////////////////////////
// Non-recursive tests
//////////////////////////////////////////////////////////////////////////////

/// Component defines begin() and end(), so BOOST_FOREACH can iterate directly
BOOST_AUTO_TEST_CASE( Iterator )
{
  Uint counter = 0;
  BOOST_FOREACH(Component& comp, root())
    BOOST_CHECK_EQUAL(comp.name(), top_component_names[counter++]);

}

BOOST_AUTO_TEST_CASE( IteratorConst )
{
  Uint counter = 0;
  BOOST_FOREACH(const Component& comp, const_root())
    BOOST_CHECK_EQUAL(comp.name(), top_component_names[counter++]);
}

/// Use a range to get all children of a certain type
BOOST_AUTO_TEST_CASE( RangeTyped )
{
  Uint counter = 0;
  BOOST_FOREACH(const Component& comp, range_typed<CGroup>(const_root()))
    BOOST_CHECK_EQUAL(comp.name(), top_group_names[counter++]);
}

/// Filtered range with type
BOOST_AUTO_TEST_CASE( RangeFilteredTyped )
{
  BOOST_FOREACH(const Component& comp, filtered_range_typed<CGroup>(const_root(), IsComponentName("group1")))
    BOOST_CHECK_EQUAL(comp.name(), "group1");
}

/// Get a named component by reference or by pointer
BOOST_AUTO_TEST_CASE( GetByName )
{
  BOOST_CHECK_THROW(get_named_component(root(), "blah"), ValueNotFound);
  BOOST_CHECK_THROW(get_named_component(root(), "group1_1"), ValueNotFound);
  BOOST_CHECK_THROW(get_named_component_typed<CGroup>(root(), "comp1"), ValueNotFound);

  BOOST_CHECK_EQUAL(get_named_component_typed<CGroup>(root(), "group1").name(), "group1");
  BOOST_CHECK_EQUAL(get_named_component_typed_ptr<CGroup>(root(), "comp1"), CGroup::Ptr());
  BOOST_CHECK_EQUAL(get_named_component_typed_ptr<CGroup>(root(), "group1")->name(), "group1");
  BOOST_CHECK_EQUAL(get_named_component_ptr(root(), "group1")->name(), "group1");
  BOOST_CHECK_EQUAL(get_named_component_ptr(root(), "group1"), root().get_child("group1"));
}

//////////////////////////////////////////////////////////////////////////////
// Recursive tests
//////////////////////////////////////////////////////////////////////////////

/// Recursive iteration over unspecified component types, using the Component interface
BOOST_AUTO_TEST_CASE( RecursiveIterator )
{
  Uint counter = 0;
  for(Component::iterator it = root().recursive_begin(); it != root().recursive_end(); ++it)
    BOOST_CHECK_EQUAL(it->name(), component_names[counter++]);
}

/// Recursive iteration over unspecified component types, using the Component interface, const
BOOST_AUTO_TEST_CASE( ConstRecursiveIterator )
{
  Uint counter = 0;
  for(Component::const_iterator it = const_root().recursive_begin(); it != const_root().recursive_end(); ++it)
    BOOST_CHECK_EQUAL(it->name(), component_names[counter++]);
}

/// Recursive iteration over typed components
BOOST_AUTO_TEST_CASE( RecursiveIteratorTyped )
{
  Uint counter = 0;
  for(ComponentIterator<CGroup> it = root().recursive_begin<CGroup>(); it != root().recursive_end<CGroup>(); ++it)
    BOOST_CHECK_EQUAL(it->name(), group_names[counter++]);
}

/// Recursive iteration over typed components, const
BOOST_AUTO_TEST_CASE( RecursiveIteratorTypedConst )
{
  Uint counter = 0;
  for(ComponentIterator<CGroup const> it = const_root().recursive_begin<CGroup>(); it != const_root().recursive_end<CGroup>(); ++it) {
    BOOST_CHECK_EQUAL(it->name(), group_names[counter++]);
  }
}

/// Manually construct a filter
BOOST_AUTO_TEST_CASE( RecursiveIteratorFiltered )
{
  typedef boost::filter_iterator< IsComponentName , Component::const_iterator > FilterIterator;
  FilterIterator filterIterator(IsComponentName("group1"), const_root().recursive_begin(), const_root().recursive_end());
  FilterIterator last_filterIterator(IsComponentName("group1"), const_root().recursive_end(), const_root().recursive_end());
  for (; filterIterator != last_filterIterator; ++filterIterator)
    BOOST_CHECK_EQUAL(filterIterator->name(),"group1");
  FilterIterator empty_begin(IsComponentName("nonExistingGroup"), const_root().recursive_begin(), const_root().recursive_end());
  FilterIterator empty_end(IsComponentName("nonExistingGroup"), const_root().recursive_end(), const_root().recursive_end());
  BOOST_CHECK_EQUAL(empty_begin == empty_end, true);
}

/// Use a range to get all children
BOOST_AUTO_TEST_CASE( RecursiveRange )
{
  Uint counter = 0;
  BOOST_FOREACH(Component& comp, recursive_range(root()))
    BOOST_CHECK_EQUAL(comp.name(), component_names[counter++]);
}

/// Use a range to get all children of a certain type
BOOST_AUTO_TEST_CASE( RecursiveRangeTyped )
{
  Uint counter = 0;
  BOOST_FOREACH(const Component& comp, recursive_range_typed<CGroup>(const_root()))
    BOOST_CHECK_EQUAL(comp.name(), group_names[counter++]);
}

/// Filtered range from iterators
BOOST_AUTO_TEST_CASE( RecursiveRangeFilteredIterators )
{
  BOOST_FOREACH(const Component& comp, make_filtered_range(root().recursive_begin(), root().recursive_end(), IsComponentName("group1")))
    BOOST_CHECK_EQUAL(comp.name(), "group1");
}

/// Filtered range from parent component
BOOST_AUTO_TEST_CASE( RecursiveRangeFiltered )
{
  BOOST_FOREACH(const Component& comp, recursive_filtered_range(const_root(), IsComponentName("group1")))
    BOOST_CHECK_EQUAL(comp.name(), "group1");
}

/// Filtered range with type
BOOST_AUTO_TEST_CASE( RecursiveRangeFilteredTyped )
{
  BOOST_FOREACH(const Component& comp, recursive_filtered_range_typed<CGroup>(const_root(), IsComponentName("group1")))
    BOOST_CHECK_EQUAL(comp.name(), "group1");
}

/// There are several ways to check if a range is empty
BOOST_AUTO_TEST_CASE( RecursiveRangeEmpty )
{
  // Use the empty method of boost::range
  BOOST_CHECK_EQUAL(recursive_filtered_range_typed<CGroup>(const_root(), IsComponentName("comp1")).empty(), true);
  // Explicitely construct a range:
  ComponentIteratorRange<Component const, CGroup, IsComponentName>::type range = recursive_filtered_range_typed<CGroup>(const_root(), IsComponentName("comp1"));
  BOOST_CHECK_EQUAL(range.begin() == range.end(), true);
}

/// Get a named component by reference or by pointer
BOOST_AUTO_TEST_CASE( GetByNameRecursive )
{
  BOOST_CHECK_THROW(recursive_get_named_component(root(), "blah"), ValueNotFound);
  BOOST_CHECK_EQUAL(recursive_get_named_component(root(), "group1_1").name(), "group1_1");
  BOOST_CHECK_THROW(recursive_get_named_component_typed<CGroup>(root(), "comp1"), ValueNotFound);
  BOOST_CHECK_EQUAL(recursive_get_named_component_typed<CGroup>(root(), "group1").name(), "group1");
  BOOST_CHECK_EQUAL(recursive_get_named_component_typed_ptr<CGroup>(root(), "comp1"), CGroup::Ptr());
  BOOST_CHECK_EQUAL(recursive_get_named_component_typed_ptr<CGroup>(root(), "group1")->name(), "group1");
  BOOST_CHECK_EQUAL(recursive_get_named_component_ptr(root(), "group1")->name(), "group1");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

