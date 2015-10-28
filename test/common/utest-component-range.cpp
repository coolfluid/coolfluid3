// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for iteration over components"

#include <iostream>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/timer.hpp>

#include "common/Log.hpp"
#include "common/Component.hpp"
#include "common/ComponentRange.hpp"
#include "common/FindComponents.hpp"
#include "common/Group.hpp"
#include "common/Link.hpp"
#include "common/OptionList.hpp"
#include "common/StringConversion.hpp"
#include "common/TaggedComponentFilter.hpp"

using namespace cf3;



//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( ComponentRangeTests )

//////////////////////////////////////////////////////////////////////////////



BOOST_AUTO_TEST_CASE( test_vector )
{
  const Uint nb_components = 10;

  auto root = common::allocate_component<common::Group>("Root");

  common::ComponentVector<common::Component> vector;

  vector.resize(nb_components);
  BOOST_CHECK_EQUAL(vector.size(), nb_components);

  for(Uint i = 0; i != nb_components; ++i)
  {
    BOOST_CHECK(vector.is_null(i));
  }

  vector.resize(0);
  vector.reserve(nb_components);

  for(Uint i = 0; i != nb_components; ++i)
  {
    vector.push_back(*root);
  }

  for(Uint i = 0; i != nb_components; ++i)
  {
    BOOST_CHECK_EQUAL(vector[i].name(), "Root");
  }

#ifdef CF3_ENABLE_CPP11
  for(common::Component& comp : vector)
  {
    BOOST_CHECK_EQUAL(comp.name(), "Root");
  }
#endif

  common::ComponentVector<common::Component const> const_vector;

  for(Uint i = 0; i != nb_components; ++i)
  {
    const_vector.push_back(*root);
  }
  BOOST_CHECK_EQUAL(const_vector.size(), nb_components);

#ifdef CF3_ENABLE_CPP11
  for(const common::Component& comp : const_vector)
  {
    BOOST_CHECK_EQUAL(comp.name(), "Root");
  }
#endif

}

BOOST_AUTO_TEST_CASE( test_range )
{
  const Uint nb_components = 6;
  const Uint nb_groups = 4;
  auto root = common::allocate_component<common::Group>("Root");

  // Check that everything is empty at the beginning
  BOOST_CHECK(common::make_range(*root).empty());
  BOOST_CHECK(common::make_range(*root).filter([](const common::Component&) {return true;}).empty());
  BOOST_CHECK(common::make_range(*root).recurse().empty());

  // Create base components
  for(Uint i = 0; i != nb_components; ++i)
  {
    root->create_component<common::Component>("comp_"+common::to_str(i));
  }

  // Create groups
  for(Uint i = 0; i != nb_groups; ++i)
  {
    root->create_component<common::Group>("group_"+common::to_str(i));
  }

  // Count components and output names
  Uint counted_components = 0;
  std::cout << "----------- begin test basic range..." << std::endl;
  for(const common::Component& comp : common::make_range(*root))
  {
    std::cout << "found component " << comp.name() << std::endl;
    ++counted_components;
  }
  BOOST_CHECK_EQUAL(counted_components, nb_components + nb_groups);
  std::cout << "...end test basic range--------------" << std::endl;

  // Count groups
  Uint counted_groups = 0;
  std::cout << "----------- begin test group range..." << std::endl;
  for(const common::Group& group : common::make_range<common::Group>(*root))
  {
    std::cout << "found group " << group.name() << std::endl;
    ++counted_groups;
  }
  BOOST_CHECK_EQUAL(counted_groups, nb_groups);
  std::cout << "...end test group range--------------" << std::endl;

  // Count links
  Uint counted_links = 0;
  for(const common::Link& link : common::make_range<common::Link>(*root))
  {
    std::cout << "found link " << link.name() << std::endl;
    ++counted_links;
  }
  BOOST_CHECK_EQUAL(counted_links, 0);

  // Filtering: components with name ending in "_2"
  std::cout << "----------- begin test single filter..." << std::endl;
  Uint filtered_components = 0;
  for(const common::Component& comp : common::make_range(*root).filter([](const common::Component& comp) {return boost::algorithm::ends_with(comp.name(), "_2");}))
  {
    std::cout << "filtered component " << comp.name() << std::endl;
    ++filtered_components;
  }
  BOOST_CHECK_EQUAL(filtered_components, 2);
  std::cout << "...end test single filter--------------" << std::endl;

  // Double filter: Groups with name ending in "_2"
  std::cout << "----------- begin test double filter..." << std::endl;
  filtered_components = 0;
  for(const common::Component& comp : common::make_range(*root).filter([](const common::Component& comp) {return boost::algorithm::ends_with(comp.name(), "_2");})
                                                               .filter([](const common::Component& comp) {return dynamic_cast<const common::Group*>(&comp) != nullptr;}))
  {
    std::cout << "double filtered component " << comp.name() << std::endl;
    ++filtered_components;
  }
  BOOST_CHECK_EQUAL(filtered_components, 1);
  std::cout << "...end test double filter--------------" << std::endl;

  // Triple filter: Groups with name ending in "_2" and starting with "comp" (should not exist)
  filtered_components = 0;
  for(const common::Component& comp : common::make_range(*root).filter([](const common::Component& comp) {return boost::algorithm::ends_with(comp.name(), "_2");})
                                                               .filter([](const common::Component& comp) {return dynamic_cast<const common::Group*>(&comp) != nullptr;})
                                                               .filter([](const common::Component& comp) {return boost::algorithm::starts_with(comp.name(), "comp");}))
  {
    std::cout << "triple filtered component " << comp.name() << std::endl;
    ++filtered_components;
  }
  BOOST_CHECK_EQUAL(filtered_components, 0);

  // Recursion without subtrees created
  std::cout << "----------- begin test recursion 1..." << std::endl;
  counted_components = 0;
  for(const common::Component& comp : common::make_range(*root).recurse())
  {
    std::cout << "recursive component " << comp.name() << std::endl;
    ++counted_components;
  }
  BOOST_CHECK_EQUAL(counted_components, nb_components + nb_groups);
  std::cout << "...end test recursion 1--------------" << std::endl;

  // Create some subgroups
  for(Uint i = 0; i != nb_components; ++i)
  {
    root->get_child("comp_1")->create_component<common::Component>("comp_"+common::to_str(i));
    root->get_child("group_2")->create_component<common::Component>("comp_"+common::to_str(i));
  }
  for(Uint i = 0; i != nb_groups; ++i)
  {
    root->get_child("comp_1")->create_component<common::Group>("group_"+common::to_str(i));
    root->get_child("group_2")->create_component<common::Group>("group_"+common::to_str(i));
  }

  root->get_child("group_2")->get_child("group_1")->create_component<common::Group>("extragroup");

  counted_components = 0;
  std::cout << "----------- begin test recursion 2..." << std::endl;
  for(const common::Component& comp : common::make_range(*root).recurse())
  {
    std::cout << "recursive component " << comp.uri().path() << std::endl;
    ++counted_components;
  }
  BOOST_CHECK_EQUAL(counted_components, 3*(nb_components + nb_groups) + 1);
  std::cout << "...end test recursion 2--------------" << std::endl;

  counted_components = 0;
  std::cout << "----------- begin test recursion 3..." << std::endl;
  for(const common::Component& comp : common::make_range(*root).filter([](const common::Component& comp) {return dynamic_cast<const common::Group*>(&comp) != nullptr;}).recurse())
  {
    std::cout << "recursive component " << comp.uri().path() << std::endl;
    ++counted_components;
  }
  BOOST_CHECK_EQUAL(counted_components, nb_components + 2*nb_groups + 1);
  std::cout << "...end test recursion 3--------------" << std::endl;

  counted_components = 0;
  std::cout << "----------- begin test recursion 4..." << std::endl;
  for(const auto& comp : common::make_range<common::Group>(*root).recurse())
  {
    std::cout << "recursive component " << comp.uri().path() << std::endl;
    ++counted_components;
  }
  BOOST_CHECK_EQUAL(counted_components, 2*nb_groups + 1);
  std::cout << "...end test recursion 4--------------" << std::endl;

  counted_components = 0;
  std::cout << "----------- begin test recursion 5..." << std::endl;
  for(const common::Component& comp : common::make_range(*root).recurse().filter([](const common::Component& comp){ return comp.name() == "extragroup"; }))
  {
    std::cout << "recursive component " << comp.uri().path() << std::endl;
    ++counted_components;
  }
  BOOST_CHECK_EQUAL(counted_components, 1);
  std::cout << "...end test recursion 5--------------" << std::endl;
}

////////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

