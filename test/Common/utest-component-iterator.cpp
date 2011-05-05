// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for iteration over components"

#include <boost/test/unit_test.hpp>
#include <boost/timer.hpp>

#include "Common/Log.hpp"
#include "Common/Component.hpp"
#include "Common/FindComponents.hpp"
#include "Common/CRoot.hpp"
#include "Common/CGroup.hpp"
#include "Common/CLink.hpp"

#include "Common/StringConversion.hpp"

using namespace CF;
using namespace CF::Common;

//////////////////////////////////////////////////////////////////////////////

struct ComponentIterationFixture
{
  /// common setup for each test case
  ComponentIterationFixture()
  {
    ExceptionManager::instance().ExceptionOutputs = false;
    ExceptionManager::instance().ExceptionDumps = false;
    ExceptionManager::instance().ExceptionAborts = false;

    m_root = CRoot::create ( "root" );
    Component::Ptr comp1 = m_root->create_component_ptr<Component>("comp1");
    top_component_names.push_back(comp1->name());
    component_names.push_back(comp1->name());
    Component::Ptr comp1_1 = comp1->create_component_ptr<Component>("comp1_1");
    component_names.push_back(comp1_1->name());
    Component::Ptr comp1_2 = comp1->create_component_ptr<Component>("comp1_2");
    component_names.push_back(comp1_2->name());
    Component::Ptr comp2   = m_root->create_component_ptr<Component>("comp2");
    top_component_names.push_back(comp2->name());
    component_names.push_back(comp2->name());
    Component::Ptr comp2_1 = comp2->create_component_ptr<Component>("comp2_1");
    component_names.push_back(comp2_1->name());
    Component::Ptr comp2_2 = comp2->create_component_ptr<Component>("comp2_2");
    component_names.push_back(comp2_2->name());
    CGroup::Ptr group1 = m_root->create_component_ptr<CGroup>("group1");
    top_component_names.push_back(group1->name());
    component_names.push_back(group1->name());
    top_group_names.push_back(group1->name());
    group_names.push_back(group1->name());
    Component::Ptr comp3 = group1->create_component_ptr<Component>("comp3");
    comp3->add_tag("special");
    component_names.push_back(comp3->name());
    special_component_names.push_back(comp3->name());
    group1_component_names.push_back(comp3->name());
    CGroup::Ptr group1_1 = group1->create_component_ptr<CGroup>("group1_1");
    component_names.push_back(group1_1->name());
    group_names.push_back(group1_1->name());
    group1_group_names.push_back(group1_1->name());
    group1_component_names.push_back(group1_1->name());
    CGroup::Ptr group1_2 = group1->create_component_ptr<CGroup>("group1_2");
    group1_2->add_tag("special");
    component_names.push_back(group1_2->name());
    group_names.push_back(group1_2->name());
    group1_group_names.push_back(group1_2->name());
    group1_component_names.push_back(group1_2->name());
    special_component_names.push_back(group1_2->name());
    CGroup::Ptr group2 = m_root->create_component_ptr<CGroup>("group2");
    top_component_names.push_back(group2->name());
    component_names.push_back(group2->name());
    top_group_names.push_back(group2->name());
    group_names.push_back(group2->name());
    CGroup::Ptr group2_1 = group2->create_component_ptr<CGroup>("group2_1");
    component_names.push_back(group2_1->name());
    group_names.push_back(group2_1->name());
    CGroup::Ptr group2_1_1 = group2_1->create_component_ptr<CGroup>("group2_1_1");
    component_names.push_back(group2_1_1->name());
    group_names.push_back(group2_1_1->name());
    group2_1_1->add_tag("very_special");
    CLink::Ptr link2 = group2->create_component_ptr<CLink>("link2");
    component_names.push_back(link2->name());
    CGroup::Ptr group3 = m_root->create_component_ptr<CGroup>("group3");
    top_component_names.push_back(group3->name());
    component_names.push_back(group3->name());
    top_group_names.push_back(group3->name());
    group_names.push_back(group3->name());
    CGroup::Ptr group3_1 = group3->create_component_ptr<CGroup>("group3_1");
    component_names.push_back(group3_1->name());
    group_names.push_back(group3_1->name());
    CLink::Ptr link1 = m_root->create_component_ptr<CLink>("link1");
    component_names.push_back(link1->name());
    top_component_names.push_back(link1->name());

    //root
    //root/comp1
    //root/comp1/comp1_1
    //root/comp1/comp1_2
    //root/comp2
    //root/comp2/comp2_1
    //root/comp2/comp2_2
    //root/group1
    //root/group1/comp3                          tag: special
    //root/group1/group1_1
    //root/group1/group1_2                       tag: special
    //root/group2
    //root/group2/group2_1
    //root/group2/group2_1/group2_1_1            tag: very_special
    //root/group2/link2
    //root/group3
    //root/group3/group3_1
    //root/link1


  }

  /// common tear-down for each test case
  ~ComponentIterationFixture()
  {
  }

  /// mutable root
  Component& root() { return *m_root; }

  /// const root
  const Component& const_root() { return *m_root; }

  CGroup& group1() { return *root().get_child_ptr("group1")->as_ptr<CGroup>(); }
  const CGroup& const_group1() { return *const_root().get_child_ptr("group1")->as_ptr<CGroup>(); }
  CGroup& group2() { return *root().get_child_ptr("group2")->as_ptr<CGroup>(); }
  const CGroup& const_group2() { return *const_root().get_child_ptr("group2")->as_ptr<CGroup>(); }
  CGroup& group3() { return *root().get_child_ptr("group3")->as_ptr<CGroup>(); }
  const CGroup& const_group3() { return *const_root().get_child_ptr("group3")->as_ptr<CGroup>(); }

  CGroup& group2_1() { return *group2().get_child_ptr("group2_1")->as_ptr<CGroup>(); }
  const CGroup& const_group2_1() { return *const_group2().get_child_ptr("group2_1")->as_ptr<CGroup>(); }

  /// list of all component names on the first level
  std::vector<std::string> top_component_names;
  /// list of all component names
  std::vector<std::string> component_names;
  /// list of all group names at the top level
  std::vector<std::string> top_group_names;
  /// list of all group names at in group1
  std::vector<std::string> group1_group_names;
  /// list of all group names at in group1
  std::vector<std::string> group1_component_names;
  /// list of all group names
  std::vector<std::string> group_names;
  /// list of all components with a tag "special"
  std::vector<std::string> special_component_names;
private:
  Component::Ptr m_root;

};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( ComponentIteration, ComponentIterationFixture )

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_find_parent )
{
  const CGroup& group2 = find_parent_component<CGroup>(const_group2_1());
  BOOST_CHECK_EQUAL(group2.full_path().string() , "cpath://root/group2");

  CRoot& root = find_parent_component<CRoot>(group2_1());
  BOOST_CHECK_EQUAL(root.full_path().string() , "cpath://root");

  Component& group22 = find_parent_component_with_filter(group2_1(),IsComponentName("group2"));
  BOOST_CHECK_EQUAL(group22.full_path().string() , "cpath://root/group2");

  CRoot& root2 = find_parent_component_with_filter<CRoot>(group2_1(),IsComponentType<CRoot>());
  BOOST_CHECK_EQUAL(root2.full_path().string() , "cpath://root");

}
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
  BOOST_FOREACH(const Component& comp, find_components<CGroup>(const_root()))
    BOOST_CHECK_EQUAL(comp.name(), top_group_names[counter++]);
}

/// Filtered range with type
BOOST_AUTO_TEST_CASE( RangeFilteredTyped )
{
  BOOST_FOREACH(const Component& comp, find_components_with_name<CGroup>(const_root(), "group1"))
    BOOST_CHECK_EQUAL(comp.name(), "group1");
}

/// Get a named component by reference or by pointer
BOOST_AUTO_TEST_CASE( GetByName )
{
  BOOST_CHECK_THROW(find_component_with_name(root(), "blah"), ValueNotFound);
  BOOST_CHECK_THROW(find_component_with_name(root(), "group1_1"), ValueNotFound);
  BOOST_CHECK_THROW(find_component_with_name<CGroup>(root(), "comp1"), ValueNotFound);

  BOOST_CHECK_EQUAL(find_component_with_name<CGroup>(root(), "group1").name(), "group1");
  BOOST_CHECK_EQUAL(find_component_ptr_with_name<CGroup>(root(), "comp1"), CGroup::Ptr());
  BOOST_CHECK_EQUAL(find_component_ptr_with_name<CGroup>(root(), "group1")->name(), "group1");
  BOOST_CHECK_EQUAL(find_component_ptr_with_name(root(), "group1")->name(), "group1");
  BOOST_CHECK_EQUAL(find_component_ptr_with_name(root(), "group1"), root().get_child_ptr("group1"));
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
  BOOST_FOREACH(Component& comp, find_components_recursively(root()))
    BOOST_CHECK_EQUAL(comp.name(), component_names[counter++]);
}

/// Use a range to get all children of a certain type
BOOST_AUTO_TEST_CASE( RecursiveRangeTyped )
{
  Uint counter = 0;
  BOOST_FOREACH(const Component& comp, find_components_recursively<CGroup>(const_root()))
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
  BOOST_FOREACH(const Component& comp, find_components_recursively_with_name(const_root(), "group1"))
    BOOST_CHECK_EQUAL(comp.name(), "group1");
}

/// Filtered range with type
BOOST_AUTO_TEST_CASE( RecursiveRangeFilteredTyped )
{
  BOOST_FOREACH(const Component& comp, find_components_recursively_with_name<CGroup>(const_root(), "group1"))
    BOOST_CHECK_EQUAL(comp.name(), "group1");
}

/// There are several ways to check if a range is empty
BOOST_AUTO_TEST_CASE( RecursiveRangeEmpty )
{
  // Use the empty method of boost::range
  BOOST_CHECK_EQUAL(find_components_recursively_with_name<CGroup>(const_root(), "comp1").empty(), true);
  // Explicitely construct a range:
  ComponentIteratorRange<Component const, CGroup, IsComponentName>::type range = find_components_recursively_with_name<CGroup>(const_root(), "comp1");
  BOOST_CHECK_EQUAL(range.begin() == range.end(), true);
}

/// Get a named component by reference or by pointer
BOOST_AUTO_TEST_CASE( GetByNameRecursive )
{
  BOOST_CHECK_THROW(find_component_recursively_with_name(root(), "blah"), ValueNotFound);
  BOOST_CHECK_EQUAL(find_component_recursively_with_name(root(), "group1_1").name(), "group1_1");
  BOOST_CHECK_THROW(find_component_recursively_with_name<CGroup>(root(), "comp1"), ValueNotFound);
  BOOST_CHECK_EQUAL(find_component_recursively_with_name<CGroup>(root(), "group1").name(), "group1");
  BOOST_CHECK_EQUAL(find_component_ptr_recursively_with_name<CGroup>(root(), "comp1"), CGroup::Ptr());
  BOOST_CHECK_EQUAL(find_component_ptr_recursively_with_name<CGroup>(root(), "group1")->name(), "group1");
  BOOST_CHECK_EQUAL(find_component_ptr_recursively_with_name(root(), "group1")->name(), "group1");
}

BOOST_AUTO_TEST_CASE( test_find_components )
{
  Uint counter;

  counter = 0;
  BOOST_FOREACH(CGroup& group, find_components<CGroup>(group1()))
    BOOST_CHECK_EQUAL(group.name(),group1_group_names[counter++]);
  BOOST_CHECK_EQUAL(counter, group1_group_names.size());

  counter = 0;
  BOOST_FOREACH(const CGroup& group, find_components<CGroup>(const_group1()))
    BOOST_CHECK_EQUAL(group.name(),group1_group_names[counter++]);
  BOOST_CHECK_EQUAL(counter, group1_group_names.size());

  counter = 0;
  BOOST_FOREACH(Component& group, find_components(group1()))
    BOOST_CHECK_EQUAL(group.name(),group1_component_names[counter++]);
  BOOST_CHECK_EQUAL(counter, group1_component_names.size());

  counter = 0;
  BOOST_FOREACH(const Component& group, find_components(const_group1()))
    BOOST_CHECK_EQUAL(group.name(),group1_component_names[counter++]);
  BOOST_CHECK_EQUAL(counter, group1_component_names.size());
}

BOOST_AUTO_TEST_CASE( test_find_components_with_filter )
{
  Uint counter;

  BOOST_CHECK_EQUAL(find_components_with_filter<CGroup>(group1(),IsComponentTag("special")).empty() , false);
  BOOST_CHECK_EQUAL(find_components_with_filter<CGroup>(const_group1(),IsComponentTag("special")).empty() , false);
  BOOST_CHECK_EQUAL(find_components_with_filter(group1(),IsComponentTag("special")).empty() , false);
  BOOST_CHECK_EQUAL(find_components_with_filter(const_group1(),IsComponentTag("special")).empty() , false);

  BOOST_FOREACH(CGroup& group, find_components_with_filter<CGroup>(group1(),IsComponentTag("special")))
    BOOST_CHECK_EQUAL(group.name(),"group1_2");

  counter = 0;
  BOOST_FOREACH(const CGroup& group, find_components_with_filter<CGroup>(const_group1(),IsComponentTag("special")))
    BOOST_CHECK_EQUAL(group.name(),"group1_2");

  counter = 0;
  BOOST_FOREACH(Component& group, find_components_with_filter(group1(),IsComponentTag("special")))
    BOOST_CHECK_EQUAL(group.name(),special_component_names[counter++]);

  counter = 0;
  BOOST_FOREACH(const Component& group, find_components_with_filter(const_group1(),IsComponentTag("special")))
    BOOST_CHECK_EQUAL(group.name(),special_component_names[counter++]);
}

BOOST_AUTO_TEST_CASE( test_find_components_with_tag )
{
  Uint counter;

  BOOST_CHECK_EQUAL(find_components_with_tag<CGroup>(group2(),"special").empty() , true);
  BOOST_CHECK_EQUAL(find_components_with_tag<CGroup>(const_group2(),"special").empty() , true);
  BOOST_CHECK_EQUAL(find_components_with_tag(group2(),"special").empty() , true);
  BOOST_CHECK_EQUAL(find_components_with_tag(const_group2(),"special").empty() , true);

  BOOST_CHECK_EQUAL(find_components_with_tag<CGroup>(group1(),"special").empty() , false);
  BOOST_CHECK_EQUAL(find_components_with_tag<CGroup>(const_group1(),"special").empty() , false);
  BOOST_CHECK_EQUAL(find_components_with_tag(group1(),"special").empty() , false);
  BOOST_CHECK_EQUAL(find_components_with_tag(const_group1(),"special").empty() , false);

  BOOST_FOREACH(CGroup& group, find_components_with_tag<CGroup>(group1(),"special"))
    BOOST_CHECK_EQUAL(group.name(),"group1_2");

  BOOST_FOREACH(const CGroup& group, find_components_with_tag<CGroup>(group1(),"special"))
    BOOST_CHECK_EQUAL(group.name(),"group1_2");

  counter = 0;
  BOOST_FOREACH(Component& group, find_components_with_tag(group1(),"special"))
    BOOST_CHECK_EQUAL(group.name(),special_component_names[counter++]);
  BOOST_CHECK_EQUAL(counter,special_component_names.size());

  counter = 0;
  BOOST_FOREACH(const Component& group, find_components_with_tag(const_group1(),"special"))
    BOOST_CHECK_EQUAL(group.name(),special_component_names[counter++]);
  BOOST_CHECK_EQUAL(counter,special_component_names.size());
}

BOOST_AUTO_TEST_CASE( test_find_components_with_name )
{
  BOOST_CHECK_EQUAL(find_components_with_name<CGroup>(group1(),"group1_2").empty() , false);
  BOOST_CHECK_EQUAL(find_components_with_name<CGroup>(const_group1(),"group1_2").empty() , false);
  BOOST_CHECK_EQUAL(find_components_with_name(group1(),"group1_2").empty() , false);
  BOOST_CHECK_EQUAL(find_components_with_name(const_group1(),"group1_2").empty() , false);

  BOOST_FOREACH(CGroup& group, find_components_with_name<CGroup>(group1(),"group1_2"))
    BOOST_CHECK_EQUAL(group.name(),"group1_2");

  BOOST_FOREACH(const CGroup& group, find_components_with_name<CGroup>(const_group1(),"group1_2"))
    BOOST_CHECK_EQUAL(group.name(),"group1_2");

  BOOST_FOREACH(Component& group, find_components_with_name(group1(),"group1_2"))
    BOOST_CHECK_EQUAL(group.name(),"group1_2");

  BOOST_FOREACH(const Component& group, find_components_with_name(const_group1(),"group1_2"))
    BOOST_CHECK_EQUAL(group.name(),"group1_2");
}

BOOST_AUTO_TEST_CASE( test_find_components_recursively )
{
  Uint counter;

  counter = 0;
  BOOST_FOREACH(CGroup& group, find_components_recursively<CGroup>(group1()))
    BOOST_CHECK_EQUAL(group.name(),group1_group_names[counter++]);
  BOOST_CHECK_EQUAL(counter, group1_group_names.size());

  counter = 0;
  BOOST_FOREACH(const CGroup& group, find_components_recursively<CGroup>(group1()))
    BOOST_CHECK_EQUAL(group.name(),group1_group_names[counter++]);
  BOOST_CHECK_EQUAL(counter, group1_group_names.size());

  counter = 0;
  BOOST_FOREACH(Component& group, find_components_recursively(group1()))
    BOOST_CHECK_EQUAL(group.name(),group1_component_names[counter++]);
  BOOST_CHECK_EQUAL(counter, group1_component_names.size());

  counter = 0;
  BOOST_FOREACH(const Component& group, find_components_recursively(const_group1()))
    BOOST_CHECK_EQUAL(group.name(),group1_component_names[counter++]);
  BOOST_CHECK_EQUAL(counter, group1_component_names.size());
}

BOOST_AUTO_TEST_CASE( test_find_components_recursively_with_filter )
{
  Uint counter;

  BOOST_CHECK_EQUAL(find_components_recursively_with_filter<CGroup>(group1(),IsComponentTag("special")).empty() , false);
  BOOST_CHECK_EQUAL(find_components_recursively_with_filter<CGroup>(const_group1(),IsComponentTag("special")).empty() , false);
  BOOST_CHECK_EQUAL(find_components_recursively_with_filter(group1(),IsComponentTag("special")).empty() , false);
  BOOST_CHECK_EQUAL(find_components_recursively_with_filter(const_group1(),IsComponentTag("special")).empty() , false);

  BOOST_FOREACH(CGroup& group, find_components_recursively_with_filter<CGroup>(group1(),IsComponentTag("special")))
    BOOST_CHECK_EQUAL(group.name(),"group1_2");

  BOOST_FOREACH(const CGroup& group, find_components_recursively_with_filter<CGroup>(const_group1(),IsComponentTag("special")))
    BOOST_CHECK_EQUAL(group.name(),"group1_2");

  counter = 0;
  BOOST_FOREACH(Component& group, find_components_recursively_with_filter(group1(),IsComponentTag("special")))
    BOOST_CHECK_EQUAL(group.name(),special_component_names[counter++]);
  BOOST_CHECK_EQUAL(counter, special_component_names.size());

  counter = 0;
  BOOST_FOREACH(const Component& group, find_components_recursively_with_filter(const_group1(),IsComponentTag("special")))
    BOOST_CHECK_EQUAL(group.name(),special_component_names[counter++]);
  BOOST_CHECK_EQUAL(counter, special_component_names.size());
}

BOOST_AUTO_TEST_CASE( test_find_components_recursively_with_tag )
{

  BOOST_CHECK_EQUAL(find_components_recursively_with_tag<CGroup>(group2(),"very_special").empty() , false);
  BOOST_CHECK_EQUAL(find_components_recursively_with_tag<CGroup>(const_group2(),"very_special").empty() , false);
  BOOST_CHECK_EQUAL(find_components_recursively_with_tag(group2(),"very_special").empty() , false);
  BOOST_CHECK_EQUAL(find_components_recursively_with_tag(const_group2(),"very_special").empty() , false);

  BOOST_FOREACH(CGroup& group, find_components_recursively_with_tag<CGroup>(group2(),"very_special"))
    BOOST_CHECK_EQUAL(group.name(),"group2_1_1");

  BOOST_FOREACH(const CGroup& group, find_components_recursively_with_tag<CGroup>(const_group2(),"very_special"))
    BOOST_CHECK_EQUAL(group.name(),"group2_1_1");

  BOOST_FOREACH(Component& group, find_components_recursively_with_tag(group2(),"very_special"))
    BOOST_CHECK_EQUAL(group.name(),"group2_1_1");

  BOOST_FOREACH(const Component& group, find_components_recursively_with_tag(const_group2(),"very_special"))
    BOOST_CHECK_EQUAL(group.name(),"group2_1_1");
}

BOOST_AUTO_TEST_CASE( test_find_components_recursively_with_name )
{
  BOOST_CHECK_EQUAL(find_components_recursively_with_name<CGroup>(group2(),"group2_1_1").empty() , false);
  BOOST_CHECK_EQUAL(find_components_recursively_with_name<CGroup>(const_group2(),"group2_1_1").empty() , false);
  BOOST_CHECK_EQUAL(find_components_recursively_with_name(group2(),"group2_1_1").empty() , false);
  BOOST_CHECK_EQUAL(find_components_recursively_with_name(const_group2(),"group2_1_1").empty() , false);

  BOOST_FOREACH(CGroup& group, find_components_recursively_with_name<CGroup>(group2(),"group2_1_1"))
    BOOST_CHECK_EQUAL(group.name(),"group2_1_1");

  BOOST_FOREACH(const CGroup& group, find_components_recursively_with_name<CGroup>(const_group2(),"group2_1_1"))
    BOOST_CHECK_EQUAL(group.name(),"group2_1_1");

  BOOST_FOREACH(Component& group, find_components_recursively_with_name(group2(),"group2_1_1"))
    BOOST_CHECK_EQUAL(group.name(),"group2_1_1");

  BOOST_FOREACH(const Component& group, find_components_recursively_with_name(const_group2(),"group2_1_1"))
    BOOST_CHECK_EQUAL(group.name(),"group2_1_1");
}

BOOST_AUTO_TEST_CASE( test_find_component )
{
  BOOST_CHECK_EQUAL(find_component(group2_1()).name() , "group2_1_1" );
  BOOST_CHECK(find_component_ptr(group2_1()) != nullptr );
  BOOST_CHECK_EQUAL(find_component(const_group2_1()).name() , "group2_1_1" );
  BOOST_CHECK(find_component_ptr(const_group2_1()) != nullptr );

  BOOST_CHECK_EQUAL(find_component<CGroup>(group2()).name() , "group2_1" );
  BOOST_CHECK(find_component_ptr<CGroup>(group2()) != nullptr );
  BOOST_CHECK_EQUAL(find_component_ptr<CGroup>(group2())->name() , "group2_1" );
  BOOST_CHECK_EQUAL(find_component<CGroup>(const_group2()).name() , "group2_1" );
  BOOST_CHECK(find_component_ptr<CGroup>(const_group2()) != nullptr );
  BOOST_CHECK_EQUAL(find_component_ptr<CGroup>(const_group2())->name() , "group2_1" );
}

BOOST_AUTO_TEST_CASE( test_find_component_with_filter )
{
  BOOST_CHECK_EQUAL(find_component_with_filter(group2(),IsComponentName("group2_1")).name() , "group2_1" );
  BOOST_CHECK(find_component_ptr_with_filter(group2(),IsComponentName("group2_1")) != nullptr );
  BOOST_CHECK_EQUAL(find_component_with_filter(const_group2(),IsComponentName("group2_1")).name() , "group2_1" );
  BOOST_CHECK(find_component_ptr_with_filter(const_group2(),IsComponentName("group2_1")) != nullptr );

  BOOST_CHECK_EQUAL(find_component_with_filter<CGroup>(group2(),IsComponentName("group2_1")).name() , "group2_1" );
  BOOST_CHECK(find_component_ptr_with_filter<CGroup>(group2(),IsComponentName("group2_1")) != nullptr );
  BOOST_CHECK_EQUAL(find_component_ptr_with_filter<CGroup>(group2(),IsComponentName("group2_1"))->name() , "group2_1" );
  BOOST_CHECK_EQUAL(find_component_with_filter<CGroup>(const_group2(),IsComponentName("group2_1")).name() , "group2_1" );
  BOOST_CHECK(find_component_ptr_with_filter<CGroup>(const_group2(),IsComponentName("group2_1")) != nullptr );
  BOOST_CHECK_EQUAL(find_component_ptr_with_filter<CGroup>(const_group2(),IsComponentName("group2_1"))->name() , "group2_1" );
}

BOOST_AUTO_TEST_CASE( test_find_component_with_name )
{
  BOOST_CHECK_EQUAL(find_component_with_name(group2(),"group2_1").name() , "group2_1" );
  BOOST_CHECK(find_component_ptr_with_name(group2(),"group2_1") != nullptr );
  BOOST_CHECK_EQUAL(find_component_with_name(const_group2(),"group2_1").name() , "group2_1" );
  BOOST_CHECK(find_component_ptr_with_name(const_group2(),"group2_1") != nullptr );

  BOOST_CHECK_EQUAL(find_component_with_name<CGroup>(group2(),"group2_1").name() , "group2_1" );
  BOOST_CHECK(find_component_ptr_with_name<CGroup>(group2(),"group2_1") != nullptr );
  BOOST_CHECK_EQUAL(find_component_ptr_with_name<CGroup>(group2(),"group2_1")->name() , "group2_1" );
  BOOST_CHECK_EQUAL(find_component_with_name<CGroup>(const_group2(),"group2_1").name() , "group2_1" );
  BOOST_CHECK(find_component_ptr_with_name<CGroup>(const_group2(),"group2_1") != nullptr );
  BOOST_CHECK_EQUAL(find_component_ptr_with_name<CGroup>(const_group2(),"group2_1")->name() , "group2_1" );
}

BOOST_AUTO_TEST_CASE( test_find_component_with_tag )
{
  BOOST_CHECK_EQUAL(find_component_with_tag(group2_1(),"very_special").name() , "group2_1_1" );
  BOOST_CHECK(find_component_ptr_with_tag(group2_1(),"very_special") != nullptr );
  BOOST_CHECK_EQUAL(find_component_with_tag(const_group2_1(),"very_special").name() , "group2_1_1" );
  BOOST_CHECK(find_component_ptr_with_tag(const_group2_1(),"very_special") != nullptr );

  BOOST_CHECK_EQUAL(find_component_with_tag<CGroup>(group1(),"special").name() , "group1_2" );
  BOOST_CHECK(find_component_ptr_with_tag<CGroup>(group1(),"special") != nullptr );
  BOOST_CHECK_EQUAL(find_component_ptr_with_tag<CGroup>(group1(),"special")->name() , "group1_2" );
  BOOST_CHECK_EQUAL(find_component_with_tag<CGroup>(const_group1(),"special").name() , "group1_2" );
  BOOST_CHECK(find_component_ptr_with_tag<CGroup>(const_group1(),"special") != nullptr );
  BOOST_CHECK_EQUAL(find_component_ptr_with_tag<CGroup>(const_group1(),"special")->name() , "group1_2" );
}

BOOST_AUTO_TEST_CASE( test_find_component_recursively )
{
  BOOST_CHECK_EQUAL(find_component_recursively(group2_1()).name() , "group2_1_1" );
  BOOST_CHECK(find_component_ptr_recursively(group2_1()) != nullptr );
  BOOST_CHECK_EQUAL(find_component_recursively(const_group2_1()).name() , "group2_1_1" );
  BOOST_CHECK(find_component_ptr_recursively(const_group2_1()) != nullptr );

  BOOST_CHECK_EQUAL(find_component_recursively<CGroup>(group2_1()).name() , "group2_1_1" );
  BOOST_CHECK(find_component_ptr_recursively<CGroup>(group2_1()) != nullptr );
  BOOST_CHECK_EQUAL(find_component_ptr_recursively<CGroup>(group2_1())->name() , "group2_1_1" );
  BOOST_CHECK_EQUAL(find_component_recursively<CGroup>(const_group2_1()).name() , "group2_1_1" );
  BOOST_CHECK(find_component_ptr_recursively<CGroup>(const_group2_1()) != nullptr );
  BOOST_CHECK_EQUAL(find_component_ptr_recursively<CGroup>(const_group2_1())->name() , "group2_1_1" );
}

BOOST_AUTO_TEST_CASE( test_find_component_recursively_with_filter )
{
  BOOST_CHECK_EQUAL(find_component_recursively_with_filter(group2(),IsComponentTag("very_special")).name() , "group2_1_1" );
  BOOST_CHECK(find_component_ptr_recursively_with_filter(group2(),IsComponentTag("very_special")) != nullptr );
  BOOST_CHECK_EQUAL(find_component_recursively_with_filter(const_group2(),IsComponentTag("very_special")).name() , "group2_1_1" );
  BOOST_CHECK(find_component_ptr_recursively_with_filter(const_group2(),IsComponentTag("very_special")) != nullptr );

  BOOST_CHECK_EQUAL(find_component_recursively_with_filter<CGroup>(group2(),IsComponentTag("very_special")).name() , "group2_1_1" );
  BOOST_CHECK(find_component_ptr_recursively_with_filter<CGroup>(group2(),IsComponentTag("very_special")) != nullptr );
  BOOST_CHECK_EQUAL(find_component_ptr_recursively_with_filter<CGroup>(group2(),IsComponentTag("very_special"))->name() , "group2_1_1" );
  BOOST_CHECK_EQUAL(find_component_recursively_with_filter<CGroup>(const_group2(),IsComponentTag("very_special")).name() , "group2_1_1" );
  BOOST_CHECK(find_component_ptr_recursively_with_filter<CGroup>(const_group2(),IsComponentTag("very_special")) != nullptr );
  BOOST_CHECK_EQUAL(find_component_ptr_recursively_with_filter<CGroup>(const_group2(),IsComponentTag("very_special"))->name() , "group2_1_1" );
}

BOOST_AUTO_TEST_CASE( test_find_component_recursively_with_name )
{
  BOOST_CHECK_EQUAL(find_component_recursively_with_name(group2(),"group2_1_1").name() , "group2_1_1" );
  BOOST_CHECK(find_component_ptr_recursively_with_name(group2(),"group2_1_1") != nullptr );
  BOOST_CHECK_EQUAL(find_component_recursively_with_name(const_group2(),"group2_1_1").name() , "group2_1_1" );
  BOOST_CHECK(find_component_ptr_recursively_with_name(const_group2(),"group2_1_1") != nullptr );

  BOOST_CHECK_EQUAL(find_component_recursively_with_name<CGroup>(group2(),"group2_1_1").name() , "group2_1_1" );
  BOOST_CHECK(find_component_ptr_recursively_with_name<CGroup>(group2(),"group2_1_1") != nullptr );
  BOOST_CHECK_EQUAL(find_component_ptr_recursively_with_name<CGroup>(group2(),"group2_1_1")->name() , "group2_1_1" );
  BOOST_CHECK_EQUAL(find_component_recursively_with_name<CGroup>(const_group2(),"group2_1_1").name() , "group2_1_1" );
  BOOST_CHECK(find_component_ptr_recursively_with_name<CGroup>(const_group2(),"group2_1_1") != nullptr );
  BOOST_CHECK_EQUAL(find_component_ptr_recursively_with_name<CGroup>(const_group2(),"group2_1_1")->name() , "group2_1_1" );
}

BOOST_AUTO_TEST_CASE( test_find_component_recursively_with_tag )
{
  BOOST_CHECK_EQUAL(find_component_recursively_with_tag(group2(),"very_special").name() , "group2_1_1" );
  BOOST_CHECK(find_component_ptr_recursively_with_tag(group2(),"very_special") != nullptr );
  BOOST_CHECK_EQUAL(find_component_recursively_with_tag(const_group2(),"very_special").name() , "group2_1_1" );
  BOOST_CHECK(find_component_ptr_recursively_with_tag(const_group2(),"very_special") != nullptr );

  BOOST_CHECK_EQUAL(find_component_recursively_with_tag<CGroup>(group2(),"very_special").name() , "group2_1_1" );
  BOOST_CHECK(find_component_ptr_recursively_with_tag<CGroup>(group2(),"very_special") != nullptr );
  BOOST_CHECK_EQUAL(find_component_ptr_recursively_with_tag<CGroup>(group2(),"very_special")->name() , "group2_1_1" );
  BOOST_CHECK_EQUAL(find_component_recursively_with_tag<CGroup>(const_group2(),"very_special").name() , "group2_1_1" );
  BOOST_CHECK(find_component_ptr_recursively_with_tag<CGroup>(const_group2(),"very_special") != nullptr );
  BOOST_CHECK_EQUAL(find_component_ptr_recursively_with_tag<CGroup>(const_group2(),"very_special")->name() , "group2_1_1" );
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( speed_find_tag )
{
    CGroup::Ptr mg = root().create_component_ptr<CGroup>("ManyGroup2");

  // allocate 5000 components
  for ( Uint i = 0; i < 250 ; ++i)
  {
    mg->create_component_ptr<CGroup>( std::string("ggg") + to_str(i) )->add_tag("CGroup");
  }

  boost::timer timer;
  Uint counter = 0;
  BOOST_FOREACH(Component& comp, find_components_recursively_with_tag(*mg, "CGroup" ) )
  {
    comp.is_link(); // to disable unused variable warning
    ++counter;
  }
  std::cout << "iterate by [tag] over " << counter << " components in " << timer.elapsed() << " seconds" << std::endl;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( speed_find_type )
{
    CGroup::Ptr mg = root().create_component_ptr<CGroup>("ManyGroup1");

  // allocate 5000 components
  for ( Uint i = 0; i < 250 ; ++i)
  {
    mg->create_component_ptr<CGroup>( std::string("ggg") + to_str(i) );
  }

  boost::timer timer;
  Uint counter = 0;
  BOOST_FOREACH(Component& comp, find_components_recursively<CGroup>(*mg) )
  {
    comp.is_link(); // to disable unused variable warning
    ++counter;
  }
  std::cout << "iterate by [type] over " << counter << " components in " << timer.elapsed() << " seconds" << std::endl;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_range_to_vector )
{
    CGroup::Ptr mg = root().create_component_ptr<CGroup>("ManyGroup1");

  // allocate 10 components
  for ( Uint i = 0; i < 10 ; ++i)
  {
    mg->create_component_ptr<CGroup>( std::string("ggg") + to_str(i) );
  }

  std::vector<CGroup::Ptr> vector = range_to_vector(find_components_recursively<CGroup>(*mg));
  BOOST_CHECK_EQUAL(vector.size() , 10u);

  CGroup::ConstPtr const_mg (mg);
  std::vector<CGroup::ConstPtr> const_vector = range_to_vector(find_components_recursively<CGroup>(*const_mg));
  BOOST_CHECK_EQUAL(const_vector.size(), 10u);

  const_vector = range_to_const_vector(find_components_recursively<CGroup>(*mg));
  BOOST_CHECK_EQUAL(const_vector.size(), 10u);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_new_range )
{
  DerivedComponentIteratorRange<Component> new_range ( root().begin(),root().end() );
  BOOST_FOREACH( Component& comp, new_range )
    CFLogVar(comp.name());

  CF_DEBUG_POINT;
  DerivedComponentIteratorRange<CGroup> new_range2 ( root().begin<CGroup>(), root().end<CGroup>() );
  BOOST_FOREACH( CGroup& comp, new_range2 )
    CFLogVar(comp.name());

  CF_DEBUG_POINT;
  DerivedComponentIteratorRange<const CGroup,IsComponentName> new_range5 ( const_root().begin<CGroup const>(), const_root().end<CGroup const>(), IsComponentName("group1") );
  BOOST_FOREACH( const CGroup& comp, new_range5 )
    CFLogVar(comp.name());

  CF_DEBUG_POINT;
  typedef DerivedComponentIteratorRange<CGroup,IsComponentName> filtered_range;
  typedef DerivedComponentIteratorRange<CGroup> group_range;

  BOOST_FOREACH( CGroup& comp, filtered_range( root().begin<CGroup>(), root().end<CGroup>() , IsComponentName("group1") ) )
    CFLogVar(comp.name());

  CFLogVar(group_range(root().begin<CGroup>(),root().end<CGroup>()).as_vector().size());
  CFLogVar(group_range(root().begin<CGroup>(),root().end<CGroup>()).as_const_vector().size());
  CFLogVar(group_range(root().begin<CGroup>(),root().end<CGroup>()).size());

  CF_DEBUG_POINT;
  BOOST_FOREACH( CGroup& comp, make_new_range(root().begin<CGroup>(),root().end<CGroup>(),IsComponentName("group1")) )
    CFLogVar(comp.name());

  BOOST_FOREACH( const CGroup& comp, make_new_range(const_root().begin<CGroup>(),const_root().end<CGroup>(),IsComponentName("group1")) )
    CFLogVar(comp.name());

  CF_DEBUG_POINT;
  DerivedConstComponentIteratorRange<CGroup> new_range3 ( const_root().begin<CGroup>(), const_root().end<CGroup>() );
  BOOST_FOREACH( const CGroup& comp, new_range3 )
    CFLogVar(comp.name());

  CF_DEBUG_POINT;
  DerivedConstComponentIteratorRange<CGroup> new_range6 ( root().begin<CGroup const>(), root().end<CGroup const>() );
  BOOST_FOREACH( const CGroup& comp, new_range6 )
    CFLogVar(comp.name());
  CFLogVar(new_range6.size());

  //
  // CF_DEBUG_POINT;
  // BOOST_FOREACH( const CGroup& comp, DerivedConstComponentIteratorRange<CGroup>(const_root()) )
  //   CFLogVar(comp.name());

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

