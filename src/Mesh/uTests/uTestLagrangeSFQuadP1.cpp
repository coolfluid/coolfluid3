#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::LagrangeSF"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CArray.hpp"
#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/LagrangeSF/QuadP1.hpp"
#include "Mesh/P1/Quad2D.hpp"

#include "Tools/Testing/Difference.hpp"

using namespace boost::assign;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Mesh::Integrators;
using namespace CF::Mesh::LagrangeSF;
using namespace CF::Mesh::P1;

//////////////////////////////////////////////////////////////////////////////

struct LagrangeSFQuadP1_Fixture
{
  typedef std::vector<RealVector> NodesT;
  /// common setup for each test case
  LagrangeSFQuadP1_Fixture() : mapped_coords(init_mapped_coords()), nodes(init_nodes()), coord(boost::extents[4][2])
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;

    coord[0][XX]=nodes[0][XX]; coord[0][YY]=nodes[0][YY];
    coord[1][XX]=nodes[1][XX]; coord[1][YY]=nodes[1][YY];
    coord[2][XX]=nodes[2][XX]; coord[2][YY]=nodes[2][YY];
    coord[3][XX]=nodes[3][XX]; coord[3][YY]=nodes[3][YY];
  }

  /// common tear-down for each test case
  ~LagrangeSFQuadP1_Fixture()
  {
  }
  /// common values accessed by all tests goes here

  const CF::RealVector mapped_coords;
  const NodesT nodes;
  CArray::Array coord;

  struct const_functor
  {
    const_functor(const NodesT& node_list) : m_nodes(node_list) {}
    template<typename GeoShapeF, typename SolShapeF>
    CF::Real valTimesDetJacobian(const CF::RealVector& mappedCoords)
    {
      return GeoShapeF::computeJacobianDeterminant(mappedCoords, m_nodes);
    }
  private:
    const NodesT& m_nodes;
  };

private:
  /// Workaround for boost:assign ambiguity
  CF::RealVector init_mapped_coords()
  {
    return list_of(0.1)(0.8);
  }

  /// Workaround for boost:assign ambiguity
  NodesT init_nodes()
  {
    const CF::RealVector c0 = list_of(0.5)(0.3);
    const CF::RealVector c1 = list_of(1.1)(1.2);
    const CF::RealVector c2 = list_of(1.35)(1.9);
    const CF::RealVector c3 = list_of(0.8)(2.1);
    return list_of(c0)(c1)(c2)(c3);
  }
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( LagrangeSFQuadP1, LagrangeSFQuadP1_Fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( computeShapeFunction )
{
  const CF::RealVector reference_result = list_of(0.045)(0.055)(0.495)(0.405);
  CF::RealVector result(4);
  QuadP1::computeShapeFunction(mapped_coords, result);
  CF::Tools::Testing::Accumulator accumulator;
  CF::Tools::Testing::vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( computeMappedCoordinates )
{
  const CF::RealVector test_coords = list_of(0.9375)(1.375); // center of the element
  CF::RealVector result(2);
  QuadP1::computeMappedCoordinates(test_coords, nodes, result);
  BOOST_CHECK_LT(std::abs(result[0]), 3e-15);
  BOOST_CHECK_LT(std::abs(result[1]), 3e-15);// sqrt from the expression gives too many ULPS in difference for Accumulator
}

BOOST_AUTO_TEST_CASE( computeMappedGradient )
{
  CF::RealMatrix expected(4, 2);
  const CF::Real ksi  = mapped_coords[0];
  const CF::Real eta = mapped_coords[1];
  expected(0, 0) = 0.25 * (-1 + eta);
  expected(0, 1) = 0.25 * (-1 + ksi);
  expected(1, 0) = 0.25 * ( 1 - eta);
  expected(1, 1) = 0.25 * (-1 - ksi);
  expected(2, 0) = 0.25 * ( 1 + eta);
  expected(2, 1) = 0.25 * ( 1 + ksi);
  expected(3, 0) = 0.25 * (-1 - eta);
  expected(3, 1) = 0.25 * ( 1 - ksi);
  CF::RealMatrix result(4, 2);
  QuadP1::computeMappedGradient(mapped_coords, result);
  CF::Tools::Testing::Accumulator accumulator;
  CF::Tools::Testing::vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( computeJacobianDeterminant )
{
  // Shapefunction determinant at center should be a quarter of the cell volume
  const CF::RealVector center_coords = list_of(0.)(0.);
  const std::vector<CArray::Row> noderows = boost::assign::list_of(coord[0])(coord[1])(coord[2])(coord[3]);
  const Real vol = CF::Mesh::VolumeComputer<Quad2D>::computeVolume(noderows);
  BOOST_CHECK_LT(boost::accumulators::max(CF::Tools::Testing::test(4.*QuadP1::computeJacobianDeterminant(center_coords, nodes), vol).ulps), 1);
}

BOOST_AUTO_TEST_CASE( computeJacobian )
{
  CF::RealMatrix expected(2, 2);
  expected(0,0) = 0.2775;
  expected(0,1) = -0.045;
  expected(1,0) = 0.13625;
  expected(1,1) = 0.5975;
  CF::RealMatrix result(2, 2);
  QuadP1::computeJacobian(mapped_coords, nodes, result);
  CF::Tools::Testing::Accumulator accumulator;
  CF::Tools::Testing::vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 15);
}

BOOST_AUTO_TEST_CASE( computeJacobianAdjoint )
{
  CF::RealMatrix expected(2, 2);
  expected(0,0) = 0.5975;
  expected(0,1) = 0.045;
  expected(1,0) = -0.13625;
  expected(1,1) = 0.2775;
  CF::RealMatrix result(2, 2);
  QuadP1::computeJacobianAdjoint(mapped_coords, nodes, result);
  CF::Tools::Testing::Accumulator accumulator;
  CF::Tools::Testing::vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 15);
}

BOOST_AUTO_TEST_CASE( integrateConst )
{
  // Shapefunction determinant should be double the volume for triangles
  const_functor ftor(nodes);
  const std::vector<CArray::Row> noderows = boost::assign::list_of(coord[0])(coord[1])(coord[2])(coord[3]);
  const Real vol = CF::Mesh::VolumeComputer<Quad2D>::computeVolume(noderows);

  CF::Real result1 = 0.0;
  CF::Real result2 = 0.0;
  CF::Real result4 = 0.0;
  CF::Real result8 = 0.0;
  CF::Real result16 = 0.0;
  CF::Real result32 = 0.0;

  Gauss<QuadP1>::integrateElement(ftor, result1);
  Gauss<QuadP1, QuadP1, 2>::integrateElement(ftor, result2);
  Gauss<QuadP1, QuadP1, 4>::integrateElement(ftor, result4);
  Gauss<QuadP1, QuadP1, 8>::integrateElement(ftor, result8);
  Gauss<QuadP1, QuadP1, 16>::integrateElement(ftor, result16);
  Gauss<QuadP1, QuadP1, 32>::integrateElement(ftor, result32);

  BOOST_CHECK_LT(boost::accumulators::max(CF::Tools::Testing::test(result1, vol).ulps), 1);
  BOOST_CHECK_LT(boost::accumulators::max(CF::Tools::Testing::test(result2, vol).ulps), 5);
  BOOST_CHECK_LT(boost::accumulators::max(CF::Tools::Testing::test(result4, vol).ulps), 5);
  BOOST_CHECK_LT(boost::accumulators::max(CF::Tools::Testing::test(result8, vol).ulps), 5);
  BOOST_CHECK_LT(boost::accumulators::max(CF::Tools::Testing::test(result16, vol).ulps), 5);
  BOOST_CHECK_LT(boost::accumulators::max(CF::Tools::Testing::test(result32, vol).ulps), 5);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

