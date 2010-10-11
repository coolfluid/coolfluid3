// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <boost/assign/list_of.hpp>

#include "Math/RealMatrix.hpp"
#include "Math/RealVector.hpp"
#include "Common/Log.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Math;
using namespace CF::Common;

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

  /// Returns the Real value 2.0. Just used to create a source for a temporary.
  Real two();
  
  /// Multiplies the given vector by two()
  CF::Math::Mult<Real, RealVector, Real> twice(const RealVector& v);


};

Real VectorOperations_Fixture::two()
{
  return 2.;
}

Mult< Real, RealVector, Real > VectorOperations_Fixture::twice(const CF::RealVector& v)
{
  return two() * v;
}



////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( VectorOperations_TestSuite, VectorOperations_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Constructors )
{
  // make a boost array
  boost::multi_array<Real,2> array (boost::extents[1][3]);
  array[0][0] = 1;
  array[0][1] = 2;
  array[0][2] = 3;
  
  // make a RealVector from a row from the boost array
  RealVector vec(array[0]);
  RealVector vec2(3);
  vec2 = RealVector(array[0]);

  // check if the vector has the correct content
  BOOST_CHECK_EQUAL(vec[0] , 1);
  BOOST_CHECK_EQUAL(vec[1] , 2);
  BOOST_CHECK_EQUAL(vec[2] , 3);
  
  BOOST_CHECK_EQUAL(vec2[0] , 1);
  BOOST_CHECK_EQUAL(vec2[1] , 2);
  BOOST_CHECK_EQUAL(vec2[2] , 3);
}

/// Inside the twice function, a temporary Real value is used. This causes the
/// stored reference to become invalid, unless the Mult ExprOp stores the constant
/// by value
BOOST_AUTO_TEST_CASE( VectorTimesTempConst )
{
  RealVector v = boost::assign::list_of(1.)(2.)(3.)(4.);
  RealVector v2(4);
  v2 = twice(v);
  
  BOOST_CHECK_CLOSE(v2[0], 2., 1e-6);
  BOOST_CHECK_CLOSE(v2[1], 4., 1e-6);
  BOOST_CHECK_CLOSE(v2[2], 6., 1e-6);
  BOOST_CHECK_CLOSE(v2[3], 8., 1e-6);
}

/// Multiplication with a matrix
BOOST_AUTO_TEST_CASE( VectorTimesMat )
{
  RealVector v = boost::assign::list_of(1.)(2.)(3.);
  RealMatrix m1(3,3,0.);
  RealMatrix m2(3,3,0.);
  
  m1(0, 0) = 0.125;
  m1(1, 1) = 0.125;
  m1(2, 2) = 0.125;
  
  m2(0, 0) = 8.;
  m2(1, 1) = 8.;
  m2(2, 2) = 8.;
  
  RealVector v2(3);
  
  // Terms should be grouped like this
  v2 = 2. * (m2 * (m1 * v));
  BOOST_CHECK_CLOSE(v2[0], 2., 1e-6);
  BOOST_CHECK_CLOSE(v2[1], 4., 1e-6);
  BOOST_CHECK_CLOSE(v2[2], 6., 1e-6);
  
  // This gives the wrong result
  v2 = 2. * m2 * (m1 * v);
  BOOST_CHECK_CLOSE(v2[0], 2., 1e-6);
  BOOST_CHECK_CLOSE(v2[1], 4., 1e-6);
  BOOST_CHECK_CLOSE(v2[2], 6., 1e-6);
}



////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

