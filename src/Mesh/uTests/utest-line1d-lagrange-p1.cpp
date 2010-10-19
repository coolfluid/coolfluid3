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

#include "Mesh/CArray.hpp"
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

struct LagrangeSFLine1DLagrangeP1Fixture
{
  typedef std::vector<RealVector> NodesT;
  /// common setup for each test case
  LagrangeSFLine1DLagrangeP1Fixture() : mapped_coords(init_mapped_coords()), nodes(init_nodes())
  {
  }

  /// common tear-down for each test case
  ~LagrangeSFLine1DLagrangeP1Fixture()
  {
  }
  /// common values accessed by all tests goes here

  const CF::RealVector mapped_coords;
  const NodesT nodes;

  struct ConstFunctor
  {
    ConstFunctor(const NodesT& node_list) : mapped_coords(3), m_nodes(node_list) {}

    Real operator()() const
    {
      return Line1DLagrangeP1::jacobian_determinant(mapped_coords, m_nodes);
    }
    RealVector mapped_coords;
  private:
    const NodesT& m_nodes;
  };

private:
  /// Workaround for boost:assign ambiguity
  CF::RealVector init_mapped_coords()
  {
    return list_of(0.2);
  }

  /// Workaround for boost:assign ambiguity
  NodesT init_nodes()
  {
    const CF::RealVector c0 = list_of(5.);
    const CF::RealVector c1 = list_of(10.);
    return list_of(c0)(c1);
  }
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Line1DLagrangeP1Suite, LagrangeSFLine1DLagrangeP1Fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Volume )
{
  ElementType::NodesT nodes_line1D(2, 1);
  nodes_line1D[0][XX] = 2.0;
  nodes_line1D[1][XX] = 1.0;
  Line1DLagrangeP1 line;
  BOOST_CHECK_EQUAL(line.computeVolume(nodes_line1D),1.);
}

BOOST_AUTO_TEST_CASE( ShapeFunction )
{
  const CF::RealVector reference_result = list_of(0.4)(0.6);
  CF::RealVector result(Line1DLagrangeP1::nb_nodes);
  Line1DLagrangeP1::shape_function(mapped_coords, result);
  Accumulator accumulator;
  vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( MappedCoordinates )
{
  const CF::RealVector test_coords = list_of(6.);
  CF::RealVector result(Line1DLagrangeP1::dimension);
  Line1DLagrangeP1::mapped_coordinates(test_coords, nodes, result);
  Accumulator acc = test(result[0], -0.6);
  BOOST_CHECK_LT(boost::accumulators::max(acc.ulps), 5);
}

BOOST_AUTO_TEST_CASE( MappedGradient )
{
  CF::RealMatrix result(Line1DLagrangeP1::dimension, Line1DLagrangeP1::nb_nodes);
  CF::RealMatrix expected(Line1DLagrangeP1::dimension, Line1DLagrangeP1::nb_nodes);
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
  CF::RealMatrix expected(Line1DLagrangeP1::dimension, Line1DLagrangeP1::dimension);
  expected(0,0) = 2.5;
  CF::RealMatrix result(Line1DLagrangeP1::dimension, Line1DLagrangeP1::dimension);
  Line1DLagrangeP1::jacobian(mapped_coords, nodes, result);
  Accumulator accumulator;
  vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( JacobianAdjoint )
{
  CF::RealMatrix expected(Line1DLagrangeP1::dimension, Line1DLagrangeP1::dimension);
  expected(0,0) = 1.;
  CF::RealMatrix result(Line1DLagrangeP1::dimension, Line1DLagrangeP1::dimension);
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

