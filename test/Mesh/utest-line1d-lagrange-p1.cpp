// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for the Line1DLagrangeP1 shapefunction"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CTable.hpp"
#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/SF/Line1DLagrangeP1.hpp"

#include "Tools/Testing/Difference.hpp"

using namespace boost::assign;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Mesh::Integrators;
using namespace CF::Mesh::SF;
using namespace CF::Tools::Testing;

//////////////////////////////////////////////////////////////////////////////

typedef Line1DLagrangeP1 SFT;

struct LagrangeSFLine1DLagrangeP1Fixture
{
  typedef SFT::NodeMatrixT NodesT;
  /// common setup for each test case
  LagrangeSFLine1DLagrangeP1Fixture() : mapped_coords((SFT::MappedCoordsT() << .2).finished()), nodes(5., 10.)
  {
  }

  /// common tear-down for each test case
  ~LagrangeSFLine1DLagrangeP1Fixture()
  {
  }
  /// common values accessed by all tests goes here

  const SFT::MappedCoordsT mapped_coords;
  const NodesT nodes;

  struct ConstFunctor
  {
    ConstFunctor(const NodesT& node_list) : m_nodes(node_list) {}

    Real operator()() const
    {
      return Line1DLagrangeP1::jacobian_determinant(mapped_coords, m_nodes);
    }
    SFT::CoordsT mapped_coords;
  private:
    const NodesT& m_nodes;
  };
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Line1DLagrangeP1Suite, LagrangeSFLine1DLagrangeP1Fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Volume )
{
  NodesT nodes_line1D(2.0, 1.0);
  Line1DLagrangeP1 line;
  BOOST_CHECK_EQUAL(line.compute_volume(nodes_line1D),1.);
}

BOOST_AUTO_TEST_CASE( ShapeFunction )
{
  const SFT::ShapeFunctionsT reference_result(0.4, 0.6);
  SFT::ShapeFunctionsT result;
  Line1DLagrangeP1::shape_function(mapped_coords, result);
  Accumulator accumulator;
  vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( MappedCoordinates )
{
  SFT::CoordsT test_coords;
  test_coords << 6.;
  SFT::MappedCoordsT result;
  Line1DLagrangeP1::mapped_coordinates(test_coords, nodes, result);
  Accumulator acc = test(result[0], -0.6);
  BOOST_CHECK_LT(boost::accumulators::max(acc.ulps), 5);
}

BOOST_AUTO_TEST_CASE( MappedGradient )
{
  SFT::MappedGradientT result;
  SFT::MappedGradientT expected;
  expected(0,0) = -0.5;
  expected(0,1) = 0.5;
  Line1DLagrangeP1::mapped_gradient(mapped_coords, result);
  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( JacobianDeterminant )
{
  BOOST_CHECK_LT(boost::accumulators::max(test(Line1DLagrangeP1::jacobian_determinant(mapped_coords, nodes), 0.5*Line1DLagrangeP1::volume(nodes)).ulps), 1);
}

BOOST_AUTO_TEST_CASE( Jacobian )
{
  SFT::JacobianT expected;
  expected << 2.5;
  SFT::JacobianT result;
  Line1DLagrangeP1::jacobian(mapped_coords, nodes, result);
  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( JacobianAdjoint )
{
  SFT::JacobianT expected;
  expected << 1.;
  SFT::JacobianT result;
  Line1DLagrangeP1::jacobian_adjoint(mapped_coords, nodes, result);
  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 1);
}

BOOST_AUTO_TEST_CASE( integrateConst )
{
  ConstFunctor ftor(nodes);
  const Real vol = Line1DLagrangeP1::volume(nodes);

  CF::Real result1 = 0.0;
  CF::Real result2 = 0.0;
  CF::Real result4 = 0.0;
  CF::Real result8 = 0.0;
  CF::Real result16 = 0.0;
  CF::Real result32 = 0.0;

  gauss_integrate<1, GeoShape::LINE>(ftor, ftor.mapped_coords, result1);
  gauss_integrate<2, GeoShape::LINE>(ftor, ftor.mapped_coords, result2);
  gauss_integrate<4, GeoShape::LINE>(ftor, ftor.mapped_coords, result4);
  gauss_integrate<8, GeoShape::LINE>(ftor, ftor.mapped_coords, result8);
  gauss_integrate<16, GeoShape::LINE>(ftor, ftor.mapped_coords, result16);
  gauss_integrate<32, GeoShape::LINE>(ftor, ftor.mapped_coords, result32);

  BOOST_CHECK_LT(boost::accumulators::max(test(result1, vol).ulps), 1);
  BOOST_CHECK_LT(boost::accumulators::max(test(result2, vol).ulps), 5);
  BOOST_CHECK_LT(boost::accumulators::max(test(result4, vol).ulps), 5);
  BOOST_CHECK_LT(boost::accumulators::max(test(result8, vol).ulps), 5);
  BOOST_CHECK_LT(boost::accumulators::max(test(result16, vol).ulps), 5);
  BOOST_CHECK_LT(boost::accumulators::max(test(result32, vol).ulps), 5);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

