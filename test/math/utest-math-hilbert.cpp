// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::mesh::HilbertNumbering"


#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"

#include "common/Core.hpp"
#include "common/Environment.hpp"

#include "math/Consts.hpp"
#include "math/Hilbert.hpp"

using namespace cf3;
using namespace cf3::math;
using namespace cf3::math::Consts;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

struct HilbertNumberingTests_Fixture
{
  /// common setup for each test case
  HilbertNumberingTests_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~HilbertNumberingTests_Fixture()
  {
  }
  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here
  int    m_argc;
  char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( HilbertNumberingTests_TestSuite, HilbertNumberingTests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init )
{
  Core::instance().initiate(m_argc,m_argv);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_hilbert_1d )
{
  RealVector1 min; min << 0.;
  RealVector1 max; max << 1.;
  BoundingBox bounding_box(min,max);

  RealVector1 point;
  Hilbert compute_hilbert_level_1(bounding_box, 1);
  point << 0.25; BOOST_CHECK_EQUAL(compute_hilbert_level_1(point), 0u);
  point << 0.75; BOOST_CHECK_EQUAL(compute_hilbert_level_1(point), 1u);

  Hilbert compute_hilbert_level_2(bounding_box, 2);
  point << 0.125; BOOST_CHECK_EQUAL(compute_hilbert_level_2(point), 0u);
  point << 0.375; BOOST_CHECK_EQUAL(compute_hilbert_level_2(point), 1u);
  point << 0.625; BOOST_CHECK_EQUAL(compute_hilbert_level_2(point), 2u);
  point << 0.875; BOOST_CHECK_EQUAL(compute_hilbert_level_2(point), 3u);

  CFinfo << "1D: 32 levels: max_key = " << Hilbert(bounding_box,32).max_key() << CFendl;
  CFinfo << "uint_max()             = " << uint_max() << CFendl;
}

BOOST_AUTO_TEST_CASE( test_hilbert_2d )
{
  BoundingBox bounding_box(RealVector2(0.,0.),RealVector2(1.,1.));
  Hilbert compute_hilbert_level_1(bounding_box, 1);
  BOOST_CHECK_EQUAL(compute_hilbert_level_1(RealVector2(0.25,0.25)), 0u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_1(RealVector2(0.25,0.75)), 1u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_1(RealVector2(0.75,0.75)), 2u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_1(RealVector2(0.75,0.25)), 3u);

  Hilbert compute_hilbert_level_2(bounding_box, 2);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector2(0.125,0.125)), 0u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector2(0.375,0.125)), 1u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector2(0.375,0.375)), 2u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector2(0.125,0.375)), 3u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector2(0.125,0.625)), 4u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector2(0.125,0.875)), 5u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector2(0.375,0.875)), 6u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector2(0.375,0.625)), 7u);

  CFinfo << "2D: 16 levels: max_key = " << Hilbert(bounding_box,16).max_key() << CFendl;
  CFinfo << "uint_max()             = " << uint_max() << CFendl;
}

BOOST_AUTO_TEST_CASE( test_hilbert_3d )
{
  BoundingBox bounding_box(RealVector3(0.,0.,0.),RealVector3(1.,1.,1.));
  Hilbert compute_hilbert_level_1(bounding_box, 1);
  BOOST_CHECK_EQUAL(compute_hilbert_level_1(RealVector3(0.25,0.25,0.25)), 0u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_1(RealVector3(0.25,0.25,0.75)), 1u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_1(RealVector3(0.75,0.25,0.75)), 2u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_1(RealVector3(0.75,0.25,0.25)), 3u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_1(RealVector3(0.75,0.75,0.25)), 4u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_1(RealVector3(0.75,0.75,0.75)), 5u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_1(RealVector3(0.25,0.75,0.75)), 6u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_1(RealVector3(0.25,0.75,0.25)), 7u);

  Hilbert compute_hilbert_level_2(bounding_box, 2);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector3(0.125,0.125,0.125)), 0u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector3(0.125,0.375,0.125)), 1u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector3(0.375,0.375,0.125)), 2u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector3(0.375,0.125,0.125)), 3u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector3(0.875,0.375,0.125)), 28);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector3(0.875,0.875,0.875)), 45);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector3(0.125,0.875,0.125)), 63);

  Hilbert compute_hilbert(bounding_box,10);
  CFinfo << "3D: 10 levels: max_key = " << Hilbert(bounding_box,10).max_key() << CFendl;
  CFinfo << "3D: 11 levels: max_key = " << Hilbert(bounding_box,11).max_key() << CFendl;
  CFinfo << "uint_max()             = " << uint_max() << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( terminate )
{
  Core::instance().terminate();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

