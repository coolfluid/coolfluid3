// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for Map component"

#include<iostream>

#include <boost/test/unit_test.hpp>

#include <coolfluid-config.hpp>

#include "common/Assertions.hpp"

//////////////////////////////////////////////////////////////////////////////

using namespace cf3;
using namespace cf3::common;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( BuildOptions )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( TestAssertions )
{
  std::cout << "Build type is " << CF3_BUILD_TYPE << std::endl;
  
  bool was_asserted = false;
  
  cf3_assert(was_asserted = true);
  
  if(std::string(CF3_BUILD_TYPE) == "RELEASE")
  {
    BOOST_CHECK(!was_asserted);
  }
  else
  {
    BOOST_CHECK(was_asserted);
  }
}

//////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

