// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::Component"

#include <boost/test/unit_test.hpp>

#include "common/BasicExceptions.hpp"
#include "common/UUCount.hpp"

using namespace cf3;
using namespace cf3::common;


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( UUCountSuite )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( UUCountCompare )
{
  UUCount a, b;
  UUCount c = a;
  
  BOOST_CHECK(a != b);
  BOOST_CHECK(b > a);
  BOOST_CHECK(a < b);
  
  BOOST_CHECK(a == c);
  BOOST_CHECK(a >= c);
  BOOST_CHECK(a <= c);
}

BOOST_AUTO_TEST_CASE( UUSerialization )
{
  UUCount a;
  UUCount b(a.string());
  BOOST_CHECK(a == b);
  
  BOOST_CHECK_THROW(UUCount("lala"), ParsingFailed);
  BOOST_CHECK_THROW(UUCount("lala:lolo"), ParsingFailed);
  BOOST_CHECK_THROW(UUCount("lala:1"), ParsingFailed);
  BOOST_CHECK_THROW(UUCount("01234567-89ab-cdef-0123-456789abcdef:lala"), ParsingFailed);
  
  UUCount c("01234567-89ab-cdef-0123-456789abcdef:1");
  BOOST_CHECK_EQUAL(c.string(), "01234567-89ab-cdef-0123-456789abcdef:1");
  
  UUCount d("");
  BOOST_CHECK(d.is_nil());
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
