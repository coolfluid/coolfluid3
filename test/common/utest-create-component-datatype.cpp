// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for DataComponentWrapper.hpp"

#include <set>
#include <boost/test/unit_test.hpp>

#include "common/CF.hpp"
#include "common/CreateComponentDataType.hpp"
#include "common/Log.hpp"
#include "common/Foreach.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace cf3;
using namespace cf3::common;

struct DataComponentWrapperFixture
{
  /// common setup for each test case
  DataComponentWrapperFixture()
  {
  }

  /// common tear-down for each test case
  ~DataComponentWrapperFixture()
  {
  }

  create_component_data_type( std::set<int> , Common_API, Component_std_set_int, "Component_std_set<integer>");
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( DataComponentWrapperTests, DataComponentWrapperFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( test_DataComponentWrapper )
{
  boost::shared_ptr<Component_std_set_int> comp_set = allocate_component<Component_std_set_int>("set");

  std::set<int>& wrapped_set = comp_set->data();

  BOOST_CHECK_EQUAL(comp_set->data().size() , 0u);

  wrapped_set.insert(5);

  BOOST_CHECK_EQUAL(comp_set->data().size() , 1u);
}

//////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

