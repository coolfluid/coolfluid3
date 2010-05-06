#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::LagrangeSF"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/LagrangeSF/QuadP1.hpp"
#include "Mesh/P1/Quad2D.hpp"
#include "Tools/Difference/Difference.hpp"

using namespace boost::assign;
using namespace CF::Mesh::Integrators;
using namespace CF::Mesh::LagrangeSF;
using namespace CF::Mesh::P1;

//////////////////////////////////////////////////////////////////////////////

struct LagrangeSFQuadP1_Fixture
{
  /// common setup for each test case
  LagrangeSFQuadP1_Fixture() : mapped_coords(init_mapped_coords()), nodes(init_nodes()), nodes_ptr(init_nodes_ptr())
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;

    //mapped_coords += 0.1, 0.8;
  }

  /// common tear-down for each test case
  ~LagrangeSFQuadP1_Fixture()
  {
  }
  /// common values accessed by all tests goes here

  const CF::RealVector mapped_coords;
  const NodesT nodes;
  const std::vector<CF::RealVector*> nodes_ptr;

  template<typename ShapeF>
  struct const_functor
  {
    const_functor(const NodesT& node_list) : m_nodes(node_list) {}
    CF::Real operator()(const CF::RealVector& mappedCoords)
    {
      return ShapeF::computeJacobianDeterminant(mappedCoords, m_nodes);
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

  /// Workaround for boost:assign ambiguity
  std::vector<CF::RealVector*> init_nodes_ptr()
  {
    return list_of(&nodes[0])(&nodes[1])(&nodes[2])(&nodes[3]);
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
  CF::Tools::Difference::Accumulator accumulator;
  CF::Tools::Difference::vector_test(result, reference_result, accumulator);
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
  CF::Tools::Difference::Accumulator accumulator;
  CF::Tools::Difference::vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( computeJacobianDeterminant )
{
  // Shapefunction determinant at center should be a quarter of the cell volume
  const CF::RealVector center_coords = list_of(0.)(0.);
  BOOST_CHECK_LT(boost::accumulators::max(CF::Tools::Difference::test(4.*QuadP1::computeJacobianDeterminant(center_coords, nodes), CF::Mesh::VolumeComputer<Quad2D>::computeVolume(nodes_ptr)).ulps), 1);
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
  CF::Tools::Difference::Accumulator accumulator;
  CF::Tools::Difference::vector_test(result, expected, accumulator);
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
  CF::Tools::Difference::Accumulator accumulator;
  CF::Tools::Difference::vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 15);
}

BOOST_AUTO_TEST_CASE( integrateConst )
{
  // Shapefunction determinant should be double the volume for triangles
  const_functor<QuadP1> ftor(nodes);
  const CF::Real vol = CF::Mesh::VolumeComputer<Quad2D>::computeVolume(nodes_ptr);

  CF::Real result1 = 0.0;
  CF::Real result2 = 0.0;
  CF::Real result4 = 0.0;
  CF::Real result8 = 0.0;
  CF::Real result16 = 0.0;
  CF::Real result32 = 0.0;

  Gauss<QuadP1>::integrate(ftor, result1);
  Gauss<QuadP1, QuadP1, 2>::integrate(ftor, result2);
  Gauss<QuadP1, QuadP1, 4>::integrate(ftor, result4);
  Gauss<QuadP1, QuadP1, 8>::integrate(ftor, result8);
  Gauss<QuadP1, QuadP1, 16>::integrate(ftor, result16);
  Gauss<QuadP1, QuadP1, 32>::integrate(ftor, result32);

  BOOST_CHECK_LT(boost::accumulators::max(CF::Tools::Difference::test(result1, vol).ulps), 1);
  BOOST_CHECK_LT(boost::accumulators::max(CF::Tools::Difference::test(result2, vol).ulps), 5);
  BOOST_CHECK_LT(boost::accumulators::max(CF::Tools::Difference::test(result4, vol).ulps), 5);
  BOOST_CHECK_LT(boost::accumulators::max(CF::Tools::Difference::test(result8, vol).ulps), 5);
  BOOST_CHECK_LT(boost::accumulators::max(CF::Tools::Difference::test(result16, vol).ulps), 5);
  BOOST_CHECK_LT(boost::accumulators::max(CF::Tools::Difference::test(result32, vol).ulps), 5);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

