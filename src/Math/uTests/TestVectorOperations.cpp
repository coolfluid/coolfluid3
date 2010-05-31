#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

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

  /// possibly common functions used on the tests below

  /// common values accessed by all tests goes here

};

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


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

