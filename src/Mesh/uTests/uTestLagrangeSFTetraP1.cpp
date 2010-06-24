#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::LagrangeSF"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CArray.hpp"
#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/LagrangeSF/TetraP1.hpp"
#include "Mesh/P1/Triag2D.hpp"

#include "Tools/Testing/Difference.hpp"

using namespace boost::assign;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Mesh::Integrators;
using namespace CF::Mesh::LagrangeSF;
using namespace CF::Mesh::P1;

//////////////////////////////////////////////////////////////////////////////

struct LagrangeSFTetraP1_Fixture
{
  typedef std::vector<RealVector> NodesT;
  /// common setup for each test case
  LagrangeSFTetraP1_Fixture() : mapped_coords(init_mapped_coords()), nodes(init_nodes()), coord(boost::extents[4][3]), volume(1.451803461048456186e-4)
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;

    coord[0][XX]=nodes[0][XX]; coord[0][YY]=nodes[0][YY]; coord[0][ZZ]=nodes[0][ZZ];
    coord[1][XX]=nodes[1][XX]; coord[1][YY]=nodes[1][YY]; coord[1][ZZ]=nodes[1][ZZ];
    coord[2][XX]=nodes[2][XX]; coord[2][YY]=nodes[2][YY]; coord[2][ZZ]=nodes[2][ZZ];
    coord[3][XX]=nodes[3][XX]; coord[3][YY]=nodes[3][YY]; coord[3][ZZ]=nodes[3][ZZ];
  }

  /// common tear-down for each test case
  ~LagrangeSFTetraP1_Fixture()
  {
  }
  /// common values accessed by all tests goes here

  const CF::RealVector mapped_coords;
  const NodesT nodes;
  CArray::Array coord;
  const Real volume;

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
    return list_of(0.1)(0.8)(0.45);
  }

  /// Workaround for boost:assign ambiguity
  NodesT init_nodes()
  {
    const CF::RealVector c0 = list_of(0.830434)(0.885201)(0.188108);
    const CF::RealVector c1 = list_of(0.89653)(0.899961)(0.297475);
    const CF::RealVector c2 = list_of(0.888273)(0.821744)(0.211428);
    const CF::RealVector c3 = list_of(0.950439)(0.904872)(0.20736);
    return list_of(c0)(c1)(c2)(c3);
  }
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( LagrangeSFTetraP1, LagrangeSFTetraP1_Fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ComputeShapeFunction )
{
  const CF::RealVector reference_result = list_of(-0.35)(0.1)(0.8)(0.45);
  CF::RealVector result(4);
  TetraP1::computeShapeFunction(mapped_coords, result);
  CF::Tools::Testing::Accumulator accumulator;
  CF::Tools::Testing::vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( ComputeMappedCoordinates )
{
  const CF::RealVector test_coords = list_of(0.92)(0.87)(0.21);
  const CF::RealVector reference_result = list_of(1.779178467272182762e-02)(4.106555656123735409e-01)(5.386286149811901902e-01);
  CF::RealVector result(3);
  TetraP1::computeMappedCoordinates(test_coords, nodes, result);
  CF::Tools::Testing::Accumulator accumulator;
  CF::Tools::Testing::vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( IntegrateConst )
{
  const_functor ftor(nodes);
  CF::Real result = 0.0;
  Gauss<TetraP1>::integrateElement(ftor, result);
  BOOST_CHECK_LT(boost::accumulators::max(CF::Tools::Testing::test(result, volume).ulps), 1);
}

BOOST_AUTO_TEST_CASE( ComputeMappedGradient )
{
  CF::RealMatrix expected(4, 3);
  expected(0,0) = -1.;
  expected(0,1) = -1.;
  expected(0,2) = -1.;
  expected(1,0) = 1.;
  expected(1,1) = 0.;
  expected(1,2) = 0.;
  expected(2,0) = 0.;
  expected(2,1) = 1.;
  expected(2,2) = 0.;
  expected(3,0) = 0.;
  expected(3,1) = 0.;
  expected(3,2) = 1.;
  CF::RealMatrix result(4, 3);
  TetraP1::computeMappedGradient(mapped_coords, result);
  CF::Tools::Testing::Accumulator accumulator;
  CF::Tools::Testing::vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( computeJacobianDeterminant )
{
  // Shapefunction determinant should be a sixth of the volume for tetrahedra
  const Real result = TetraP1::computeJacobianDeterminant(mapped_coords, nodes)/6.;
  BOOST_CHECK_LT(boost::accumulators::max(CF::Tools::Testing::test(result, volume).ulps), 5);
}

BOOST_AUTO_TEST_CASE( computeJacobian )
{
  CF::RealMatrix expected(3, 3);
  expected(0,0) = 6.609600000000004361e-02;
  expected(0,1) = 1.475999999999999535e-02;
  expected(0,2) = 1.093669999999999920e-01;
  expected(1,0) = 5.783899999999997377e-02;
  expected(1,1) = -6.345699999999998564e-02;
  expected(1,2) = 2.332000000000000739e-02;
  expected(2,0) = 1.200050000000000283e-01;
  expected(2,1) = 1.967099999999999405e-02;
  expected(2,2) = 1.925199999999999134e-02;
  CF::RealMatrix result(3, 3);
  TetraP1::computeJacobian(mapped_coords, nodes, result);
  CF::Tools::Testing::Accumulator accumulator;
  CF::Tools::Testing::vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( computeJacobianAdjoint )
{
  CF::RealMatrix expected(3, 3);
  expected(0,0) = -1.680401883999999273e-03;
  expected(0,1) = 1.867198736999999475e-03;
  expected(0,2) = 7.284304918999997755e-03;
  expected(1,0) = 1.685000172000002431e-03;
  expected(1,1) = -1.185210664300000161e-02;
  expected(1,2) = 4.784319192999994877e-03;
  expected(2,0) = 8.752908253999998334e-03;
  expected(2,1) = 4.710993839999994925e-04;
  expected(2,2) = -5.047957512000001042e-03;
  CF::RealMatrix result(3, 3);
  TetraP1::computeJacobianAdjoint(mapped_coords, nodes, result);
  CF::Tools::Testing::Accumulator accumulator;
  CF::Tools::Testing::vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

