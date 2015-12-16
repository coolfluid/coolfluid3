// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"

#include "math/MatrixTypes.hpp"

using namespace std;
using namespace boost;
using namespace cf3;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

struct VectorOperations_Fixture
{
  /// common setup for each test case
  VectorOperations_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~VectorOperations_Fixture()
  {
  }
};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( VectorOperations_TestSuite, VectorOperations_Fixture )

////////////////////////////////////////////////////////////////////////////////

/// Multiplication with a matrix
BOOST_AUTO_TEST_CASE( VectorTimesMat )
{
  RealVector3 v(1., 2., 3.);
  RealMatrix m1(3,3);
  RealMatrix m2(3,3);
  
  m1.setZero();
  m2.setZero();
  
  m1(0, 0) = 0.125;
  m1(1, 1) = 0.125;
  m1(2, 2) = 0.125;
  
  m2(0, 0) = 8.;
  m2(1, 1) = 8.;
  m2(2, 2) = 8.;
  
  RealVector v2(3);
  
  // Test grouped matrix calculations
  v2 = 2. * (m2 * (m1 * v));
  BOOST_CHECK_CLOSE(v2[0], 2., 1e-6);
  BOOST_CHECK_CLOSE(v2[1], 4., 1e-6);
  BOOST_CHECK_CLOSE(v2[2], 6., 1e-6);
  
  // Test lazy writing of the expression
  v2 = 2. * m2 * (m1 * v);

  BOOST_CHECK_CLOSE(v2[0], 2., 1e-6);
  BOOST_CHECK_CLOSE(v2[1], 4., 1e-6);
  BOOST_CHECK_CLOSE(v2[2], 6., 1e-6);
  
    // Test lazy writing of the expression
  v2 = 2. * m2 * m1 * v;

  BOOST_CHECK_CLOSE(v2[0], 2., 1e-6);
  BOOST_CHECK_CLOSE(v2[1], 4., 1e-6);
  BOOST_CHECK_CLOSE(v2[2], 6., 1e-6);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

