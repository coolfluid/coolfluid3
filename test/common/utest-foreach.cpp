// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for simplifying for loops"

#include <boost/test/unit_test.hpp>

#include "common/CF.hpp"
#include "common/Foreach.hpp"

//////////////////////////////////////////////////////////////////////////////

struct ForeachFixture
{
  /// common setup for each test case
  ForeachFixture()
  {
  }

  /// common tear-down for each test case
  ~ForeachFixture()
  {
  }

private:

};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( ForeachTests, ForeachFixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( test_boost_foreach )
{
  std::vector<int> vec(5,1);

  boost_foreach(int value, vec)
    BOOST_CHECK_EQUAL(value,1);
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( test_foreach_container )
{
  std::map<std::string,int> my_map;
  my_map["first"] = 1;
  my_map["second"] = 2;

  foreach_container((const std::string& key)(const int value), my_map)
  {
    if (key == "first")
      BOOST_CHECK_EQUAL(value,1);
    else if (key == "second")
      BOOST_CHECK_EQUAL(value,2);
    else
      BOOST_CHECK(false);
  }
}

//////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

