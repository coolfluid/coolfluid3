#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "Math/MatrixInverter.hpp"
#include "Math/SVDInverter.hpp"
#include "Common/Log.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Math;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

struct MatrixInverter_Fixture
{
  /// common setup for each test case
  MatrixInverter_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~MatrixInverter_Fixture()
  {
  }

  /// possibly common functions used on the tests below

  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( MatrixInverter_TestSuite, MatrixInverter_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( SVDInverterTest )
{
  //CFinfo << "testing SVDMatrixInverter " << CFflush;
  // Exact solution is taken from http://www.mathematics-online.org/inhalt/beispiel/beispiel565/
  // System Matrix
  RealMatrix A(4,3,0.0);
  A(0,0) = 2.;    A(0,1) = -4.;   A(0,2) = 5.;
  A(1,0) = 6.;    A(1,1) = 0.;    A(1,2) = 3.;
  A(2,0) = 2.;    A(2,1) = -4.;   A(2,2) = 5.;
  A(3,0) = 6.;    A(3,1) = 0.;    A(3,2) = 3.;

  // Right Hand Side
  RealVector b(0.0,4);
  b[0] = 1.;
  b[1] = 3.;
  b[2] = -1.;
  b[3] = 3.;
  
  // Known correct solution
  RealVector xCheck(0.0,3);
  xCheck[0] = 2.;
  xCheck[1] = 1.;
  xCheck[2] = 0.;
  xCheck /= 4.;
  
  // Solution to be obtained
  RealVector x(0.0,3);
  RealVector tol(1.0,3); // 1% tolerance

  // SVDInverter which decomposes in constructor
  // Check if solution matches
  SVDInverter SVD1(A);
  SVD1.solve(b,x);
  for (Uint i=0; i<x.size(); i++) {
    BOOST_CHECK_CLOSE(x[i]+1.,xCheck[i]+1.,tol[i]);
  }
  
  // SVDInverter which just allocates in constructor
  // Check if solution matches
  SVDInverter SVD2(4,3);
  x=0.;
  SVD2.solve(A,b,x);
  for (Uint i=0; i<x.size(); i++) {
    BOOST_CHECK_CLOSE(x[i]+1.,xCheck[i]+1.,tol[i]);
  }
  
  // Check if Diagonal S matches
  RealVector  s(3), sCheck(3);
  sCheck[0] = 12.;
  sCheck[1] = 6.;
  sCheck[2] = 0.;
  
  SVD2.decompose(A);
  s = SVD2.getS();
  for (Uint i=0; i<s.size(); i++) {
    BOOST_CHECK_CLOSE(s[i]+1.,sCheck[i]+1.,tol[i]);
  }


  // Check if inverse matrix matches
  RealMatrix invACheck(3,4,0.0);
  invACheck(0,0) = -2.;   invACheck(0,1) = 6.;    invACheck(0,2) = -2.;   invACheck(0,3) = 6.;
  invACheck(1,0) = -5.;   invACheck(1,1) = 3.;    invACheck(1,2) = -5.;   invACheck(1,3) = 3.;
  invACheck(2,0) = 4.;    invACheck(2,1) = 0.;    invACheck(2,2) = 4.;    invACheck(2,3) = 0.;
  invACheck /= 72.;

  RealMatrix invA;
  SVD2.invert(A,invA);
  for (Uint i=0; i<invA.nbRows(); i++)
    for (Uint j=0; j<invA.nbCols(); j++) 
  {
      BOOST_CHECK_CLOSE(invA(i,j)+1.,invACheck(i,j)+1.,tol[0]);
  }
  
}


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

