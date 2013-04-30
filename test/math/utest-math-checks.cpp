// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"

#include "math/Checks.hpp"

using namespace std;
using namespace boost;

using namespace cf3;
using namespace cf3::common;
using namespace cf3::math;

typedef long int lint;

// This function is similar to what is implemented in FloatingPoint class
// It is based on the original article "Comparison of Floating Point Numbers" by Bruce Dawson
// It was adapted to use double precision (a limitation that FloatingPointer does not have)
bool AlmostEqual2sComplement(double A, double B, uint64_t maxUlps)
{
    // Make sure maxUlps is non-negative and small enough that the
    // default NAN won't compare as equal to anything.
    //    assert(maxUlps > 0 && maxUlps < 4 * 1024 * 1024);

    lint aInt = *(lint*)&A;
    // Make aInt lexicographically ordered as a twos-complement int
    if (aInt < 0)
        aInt = 0x80000000 - aInt;
    // Make bInt lexicographically ordered as a twos-complement int
    lint bInt = *(lint*)&B;
    if (bInt < 0)
        bInt = 0x80000000 - bInt;
    lint intDiff = (aInt >= bInt) ? (aInt - bInt) : (bInt - aInt);

//    std::cout << "diff [" << intDiff << "]" << std::endl;

    if (intDiff <= maxUlps)
        return true;
    return false;
}


BOOST_AUTO_TEST_SUITE( math_checks_test_suite )

#define ULP 4

// when a real is around 1. then Consts::eps() is
// equivalent to the jump to the nearest representable real

BOOST_AUTO_TEST_CASE( floating_point_far_from_zero )
{
  std::cout.setf(ios::scientific,ios::floatfield);
  std::cout.precision(24);

  const Real a = 1.;
  const Real b = a + Consts::eps();

  std::cout << "a [" << a << "] & [" << *(uint64_t*)&a << "]" << std::endl;
  std::cout << "b [" << b << "] & [" << *(uint64_t*)&b << "]" << std::endl;

  std::cout << "diff [" << FloatingPoint<Real>(a).diff( FloatingPoint<Real>(b) ) << "]" << std::endl;

  if( FloatingPoint<Real>(a).almost_equals( FloatingPoint<Real>(b), ULP ) )
    std::cout << "FP says a and b are equals" << std::endl;
  else
    std::cout << "FP says a and b are different" << std::endl;

  if( AlmostEqual2sComplement(a,b, ULP) )
    std::cout << "C2 says a and b are equals" << std::endl;
  else
    std::cout << "C2 says a and b are different" << std::endl;

//  BOOST_CHECK_CLOSE( a, b, 0.000001); // in percentage %

  std::cout << "-----------------------------------" << std::endl;

}

BOOST_AUTO_TEST_CASE( floating_point_far_from_zero_2 )
{
  std::cout.setf(ios::scientific,ios::floatfield);
  std::cout.precision(24);

  const Real a  = 10E10;
  const lint ai = *(lint*)&a;

  const lint bi = ai+1;
  const Real b  =  *(Real*)&bi;

  std::cout << "a [" << a << "] & [" << *(uint64_t*)&a << "]" << std::endl;
  std::cout << "b [" << b << "] & [" << *(uint64_t*)&b << "]" << std::endl;

  std::cout << "diff [" << FloatingPoint<Real>(a).diff( FloatingPoint<Real>(b) ) << "]" << std::endl;

  if( FloatingPoint<Real>(a).almost_equals( FloatingPoint<Real>(b), ULP ) )
    std::cout << "FP says a and b are equals" << std::endl;
  else
    std::cout << "FP says a and b are different" << std::endl;

  if( AlmostEqual2sComplement(a,b, ULP) )
    std::cout << "C2 says a and b are equals" << std::endl;
  else
    std::cout << "C2 says a and b are different" << std::endl;

//  BOOST_CHECK_CLOSE( a, b, 0.000001); // in percentage %

  std::cout << "-----------------------------------" << std::endl;

}

BOOST_AUTO_TEST_CASE( floating_point_near_zero )
{
  std::cout.setf(ios::scientific,ios::floatfield);
  std::cout.precision(24);

  const Real a  = 0.0;
  const lint ai = *(lint*)&a;

  const lint bi = ai+1;
//  const Real b  =  *(Real*)&bi;

//  const Real b = Consts::real_min();                 //             2.2..E-308
//  const Real b = Consts::eps();                      // 2.2..E-16
  const Real b = Consts::eps() * Consts::real_min();   // 2.2..E-16 * 2.2..E-308

  std::cout << "a [" << a << "] & [" << *(uint64_t*)&a << "]" << std::endl;
  std::cout << "b [" << b << "] & [" << *(uint64_t*)&b << "]" << std::endl;

  std::cout << "diff [" << FloatingPoint<Real>(a).diff( FloatingPoint<Real>(b) ) << "]" << std::endl;

  if( FloatingPoint<Real>(a).almost_equals( FloatingPoint<Real>(b), ULP ) )
    std::cout << "FP says a and b are equals" << std::endl;
  else
    std::cout << "FP says a and b are different" << std::endl;

  if( AlmostEqual2sComplement(a,b, ULP) )
    std::cout << "C2 says a and b are equals" << std::endl;
  else
    std::cout << "C2 says a and b are different" << std::endl;

//  BOOST_CHECK_CLOSE( a, b, 0.000001); // in percentage %

  std::cout << "-----------------------------------" << std::endl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()
