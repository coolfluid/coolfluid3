// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test function parser"

#include <boost/test/unit_test.hpp>

#include <boost/assign/list_of.hpp>

#include "math/VectorialFunction.hpp"

using namespace std;
using namespace boost;
using namespace cf3;
using namespace cf3::math;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

struct FunctionParser_Fixture
{
  FunctionParser_Fixture() {}

  ~FunctionParser_Fixture() {}
};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( FunctionParser_TestSuite, FunctionParser_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Constructors )
{
  FunctionParser fp;
  fp.Parse("sqrt(x*x + y*y)", "x,y");
  double variables[2] = { 1.5, 2.9 };
  double result = fp.Eval(variables);

  BOOST_CHECK_CLOSE( result, sqrt(1.5*1.5 + 2.9*2.9) , 1e-6);
}

BOOST_AUTO_TEST_CASE( tt )
{
  FunctionParser fp;
  fp.Parse("x/y", "x,y");
  double variables[2] = { 1.5, 2.9 };
  double result = fp.Eval(variables);

  BOOST_CHECK_CLOSE( result, 1.5/2.9 , 1e-6);
}

BOOST_AUTO_TEST_CASE( function_1 )
{
  cf3::math::VectorialFunction f ("[x/y]","x,y");

  RealVector r(1);
  cf3::math::VectorialFunction::VariablesT u = boost::assign::list_of(1.0)(3.0);

  r = f(u);

  BOOST_CHECK_CLOSE( r[0], 1./3. , 1e-6);

}

BOOST_AUTO_TEST_CASE( function_2 )
{
  cf3::math::VectorialFunction f ("[x+y][5*z]","x,y,z");

  RealVector r(2);
  cf3::math::VectorialFunction::VariablesT u = boost::assign::list_of(2.0)(3.0)(7.0);

  r = f(u);

  BOOST_CHECK_CLOSE( r[0], 5.0 , 1e-6);
  BOOST_CHECK_CLOSE( r[1], 35.0 , 1e-6);

}



////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

