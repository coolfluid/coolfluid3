// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
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

struct Triag2DLagrangeP2BFixture
{
  typedef std::vector<RealVector> NodesT;
  /// common setup for each test case
  Triag2DLagrangeP2BFixture() : mapped_coords(init_mapped_coords()), nodes(init_nodes()) {}

  /// common tear-down for each test case
  ~Triag2DLagrangeP2BFixture()  {}

  /// common values accessed by all tests goes here
  const CF::RealVector mapped_coords;
  const NodesT nodes;

private:
  /// Workaround for boost:assign ambiguity
  CF::RealVector init_mapped_coords()
  {
    return list_of(1./6.)(1./6.);
  }

  /// Workaround for boost:assign ambiguity
  NodesT init_nodes()
  {
    const CF::RealVector c0 = list_of( 1.0)(0.0);
    const CF::RealVector c1 = list_of( 0.0)(2.0);
    const CF::RealVector c2 = list_of(-1.0)(0.0);
    const CF::RealVector c3 = list_of( 0.5)(1.0);
    const CF::RealVector c4 = list_of(-0.5)(1.0);
    const CF::RealVector c5 = list_of( 0.0)(0.0);
    const CF::RealVector c6 = list_of( 0.0)(2./3.);
    return list_of(c0)(c1)(c2)(c3)(c4)(c5)(c6);
  }
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Triag2DLagrangeP2BSuite, Triag2DLagrangeP2BFixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ShapeFunction )
{
  const CF::RealVector reference_result = list_of(5./18.)(-1./18.)(-1./18.)(2./9.)(-1./9.)(2./9.)(0.5);
  CF::RealVector result(Triag2DLagrangeP2B::nb_nodes);
  Triag2DLagrangeP2B::shape_function(mapped_coords, result);

  std::cout<< reference_result << std::endl;
  std::cout<< reference_result.sum() << std::endl;

  std::cout<< result << std::endl;
  std::cout<< result.sum() << std::endl;

  CF::Tools::Testing::Accumulator accumulator;
  CF::Tools::Testing::vector_test(result, reference_result, accumulator);

  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( Gradient )
{
//  const CF::RealVector reference_result = list_of(5./18.)(-1./18.)(-1./18.)(2./9.)(-1./9.)(2./9.)(0.5);
  CF::RealMatrix result ( 2, Triag2DLagrangeP2B::nb_nodes);
  Triag2DLagrangeP2B::mapped_gradient(mapped_coords, result);

//  std::cout<< reference_result << std::endl;
//  std::cout<< reference_result.sum() << std::endl;

  std::cout<< result << std::endl;

  std::cout<< result(XX,0) + result(XX,1) + result(XX,2) + result(XX,3) + result(XX,4) + result(XX,5) + result(XX,6)  << std::endl;
  std::cout<< result(YY,0) + result(YY,1) + result(YY,2) + result(YY,3) + result(YY,4) + result(YY,5) + result(YY,6)  << std::endl;


  RealVector m1 (2); m1[KSI] = 1./6.; m1[ETA] = 5./6.;
  Triag2DLagrangeP2B::mapped_gradient(m1, result);

  std::cout<< result << std::endl;

  std::cout<< result(XX,0) + result(XX,1) + result(XX,2) + result(XX,3) + result(XX,4) + result(XX,5) + result(XX,6)  << std::endl;
  std::cout<< result(YY,0) + result(YY,1) + result(YY,2) + result(YY,3) + result(YY,4) + result(YY,5) + result(YY,6)  << std::endl;

  //  CF::Tools::Testing::Accumulator accumulator;
//  CF::Tools::Testing::vector_test(result, reference_result, accumulator);

//  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

