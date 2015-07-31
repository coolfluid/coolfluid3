// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for iteration over components"

#include <iostream>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/test/unit_test.hpp>
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
  for(const common::Component& comp : common::make_range(*root))
  {
    std::cout << "found component " << comp.name() << std::endl;
    ++counted_components;
  }
  BOOST_CHECK_EQUAL(counted_components, nb_components + nb_groups);

  // Count groups
  Uint counted_groups = 0;
  for(const common::Group& group : common::make_range<common::Group>(*root))
  {
    std::cout << "found group " << group.name() << std::endl;
    ++counted_groups;
  }
  BOOST_CHECK_EQUAL(counted_groups, nb_groups);

  // Count links
  Uint counted_links = 0;
  for(const common::Link& link : common::make_range<common::Link>(*root))
  {
    std::cout << "found link " << link.name() << std::endl;
    ++counted_links;
  }
  BOOST_CHECK_EQUAL(counted_links, 0);

  // Filtering: components with name ending in "_2"
  Uint filtered_components = 0;
  for(const common::Component& comp : common::make_range(*root).filter([](const common::Component& comp) {return boost::algorithm::ends_with(comp.name(), "_2");}))
  {
    std::cout << "filtered component " << comp.name() << std::endl;
    ++filtered_components;
  }
  BOOST_CHECK_EQUAL(filtered_components, 2);

  // Double filter: Groups with name ending in "_2"
  filtered_components = 0;
  for(const common::Component& comp : common::make_range(*root).filter([](const common::Component& comp) {return boost::algorithm::ends_with(comp.name(), "_2");})
                                                               .filter([](const common::Component& comp) {return dynamic_cast<const common::Group*>(&comp) != nullptr;}))
  {
    std::cout << "double filtered component " << comp.name() << std::endl;
    ++filtered_components;
  }
  BOOST_CHECK_EQUAL(filtered_components, 1);

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
}

////////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

