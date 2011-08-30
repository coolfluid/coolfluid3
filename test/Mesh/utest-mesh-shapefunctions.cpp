// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh interpolation"

#include <boost/test/unit_test.hpp>
#include "Common/Log.hpp"
#include "Math/MatrixTypes.hpp"

using namespace CF;
using namespace CF::Common;


////////////////////////////////////////////////////////////////////////////////

class ShapeFunction
{
public:
  virtual RealRowVector value(const RealVector& local_coord) const = 0;

};

////////////////////////////////////////////////////////////////////////////////

template <typename SFType>
class ShapeFunctionT : public ShapeFunction
{
public:
  virtual RealRowVector value(const RealVector& local_coord) const
  {
    return SFType::value( local_coord );
  }
};

////////////////////////////////////////////////////////////////////////////////

class LineLagrangeP1 : public ShapeFunctionT<LineLagrangeP1>
{
public:
  typedef Eigen::Matrix<Real, 2, 1> MappedCoordsT;
  typedef Eigen::Matrix<Real, 1, 2> ValueT;
  static ValueT value(const MappedCoordsT& local_coord)
  {
    return ( ValueT() << -1., 1. ).finished();
  }
};

////////////////////////////////////////////////////////////////////////////////

struct Test_ShapeFunction_Fixture
{
  /// common setup for each test case
  Test_ShapeFunction_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~Test_ShapeFunction_Fixture()
  {
  }

  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Test_ShapeFunction_TestSuite, Test_ShapeFunction_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( static_version )
{

  LineLagrangeP1::ValueT values = LineLagrangeP1::value( (LineLagrangeP1::MappedCoordsT() << 0 , 1 ).finished() );

  std::cout << "static : values = " << values << std::endl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( dynamic_version )
{
  ShapeFunction* sf = new LineLagrangeP1;

  RealVector local_coords(2);
  local_coords << 0 , 1;

  RealRowVector values = sf->value(local_coords);

  std::cout << "dynamic: values = " << values << std::endl;

}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

