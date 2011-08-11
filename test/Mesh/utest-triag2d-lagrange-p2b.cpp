// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for Triag2DLagrangeP2B"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"

#include "Mesh/SF/Triag2DLagrangeP2B.hpp"

#include "Tools/Testing/Difference.hpp"

using namespace boost::assign;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Mesh::SF;

//////////////////////////////////////////////////////////////////////////////

typedef Triag2DLagrangeP2B SFT;

struct Triag2DLagrangeP2BFixture
{
  typedef SFT::NodeMatrixT NodesT;
  /// common setup for each test case
  Triag2DLagrangeP2BFixture() :
    mapped_coords(1./6., 1./6.),
    nodes((NodesT() <<
           1.0, 0.0,
           0.0, 2.0,
           -1.0, 0.0,
           0.5, 1.0,
           -0.5, 1.0,
           0.0, 0.0,
           0.0, 2./3.).finished())
  {}

  /// common tear-down for each test case
  ~Triag2DLagrangeP2BFixture()  {}

  /// common values accessed by all tests goes here
  const SFT::MappedCoordsT mapped_coords;
  const NodesT nodes;

};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Triag2DLagrangeP2BSuite, Triag2DLagrangeP2BFixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ShapeFunction )
{
  SFT::ShapeFunctionsT reference_result;
  reference_result << 5./18., -1./18., -1./18., 2./9., -1./9., 2./9., 0.5;
  SFT::ShapeFunctionsT result;
  Triag2DLagrangeP2B::shape_function_value(mapped_coords, result);

  std::cout<< reference_result << std::endl;
  std::cout<< reference_result.sum() << std::endl;

  std::cout<< result << std::endl;
  std::cout<< result.sum() << std::endl;


  BOOST_CHECK_CLOSE ( result.sum(), 1.0, 1e-8    );

}

BOOST_AUTO_TEST_CASE( Gradient )
{
//  const CF::RealVector reference_result = list_of(5./18.)(-1./18.)(-1./18.)(2./9.)(-1./9.)(2./9.)(0.5);
  SFT::MappedGradientT result;
  Triag2DLagrangeP2B::shape_function_gradient(mapped_coords, result);

//  std::cout<< reference_result << std::endl;
//  std::cout<< reference_result.sum() << std::endl;

  std::cout<< result << std::endl;

  std::cout<< result(XX,0) + result(XX,1) + result(XX,2) + result(XX,3) + result(XX,4) + result(XX,5) + result(XX,6)  << std::endl;
  std::cout<< result(YY,0) + result(YY,1) + result(YY,2) + result(YY,3) + result(YY,4) + result(YY,5) + result(YY,6)  << std::endl;


  const SFT::MappedCoordsT m1(1./6.,  5./6.);
  Triag2DLagrangeP2B::shape_function_gradient(m1, result);

  std::cout<< result << std::endl;

  std::cout<< result(XX,0) + result(XX,1) + result(XX,2) + result(XX,3) + result(XX,4) + result(XX,5) + result(XX,6)  << std::endl;
  std::cout<< result(YY,0) + result(YY,1) + result(YY,2) + result(YY,3) + result(YY,4) + result(YY,5) + result(YY,6)  << std::endl;

  BOOST_CHECK_LE ( std::abs(result.sum()), 1e-14 );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

