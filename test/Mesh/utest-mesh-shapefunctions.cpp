// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Proposition for new element / shapefunction API"

#include <boost/test/unit_test.hpp>
#include "Common/Log.hpp"
#include "Common/Component.hpp"

#include "Mesh/ShapeFunction.hpp"
#include "Mesh/LagrangeP0/Triag.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;

////////////////////////////////////////////////////////////////////////////////

struct Test_ShapeFunction_Fixture
{
  /// common setup for each test case
  Test_ShapeFunction_Fixture()
  {
  }

  /// common tear-down for each test case
  ~Test_ShapeFunction_Fixture()
  {
  }
};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Test_ShapeFunction_TestSuite, Test_ShapeFunction_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( static_version )
{
  LagrangeP0::Triag::MappedCoordsT mapped_coord = (LagrangeP0::Triag::MappedCoordsT() << 0, 0 ).finished();

  LagrangeP0::Triag::ValueT values       = LagrangeP0::Triag::value( mapped_coord );
  const RealMatrix&             local_coords = LagrangeP0::Triag::local_coordinates();

  std::cout << "static : values       = " << values       << std::endl;
  std::cout << "static : local_coords = " << local_coords << std::endl;

  LagrangeP0::Triag::compute_value(mapped_coord, values);
  std::cout << "static : values       = " << values       << std::endl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( dynamic_version )
{
  RealVector mapped_coord = (RealVector(2) << 0, 0).finished();

  std::auto_ptr<ShapeFunction> sf (new LagrangeP0::Triag);

  RealRowVector     values       = sf->value(mapped_coord);
  const RealMatrix& local_coords = sf->local_coordinates();

  std::cout << "dynamic: values       = " << values       << std::endl;
  std::cout << "dynamic: local_coords = " << local_coords << std::endl;

  sf->compute_value(mapped_coord,values);
  std::cout << "dynamic: values       = " << values       << std::endl;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

