// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for Eigen"

#include <boost/test/unit_test.hpp>

/// fixture for each test of Eigen
struct EigenFixture
{
  EigenFixture() {}
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( TestEigen, EigenFixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Matrix )
{
//  BOOST_CHECK( bool );
//  BOOST_CHECK_EQUAL(str, f.m_buffer);
//  BOOST_CHECK_CLOSE( v1, v2, 0.0001 );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
