#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for Triag2DLagrangeP1"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CArray.hpp"
#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/SF/Triag2DLagrangeP1.hpp"

#include "Tools/Testing/Difference.hpp"

using namespace boost::assign;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Mesh::Integrators;
using namespace CF::Mesh::SF;

//////////////////////////////////////////////////////////////////////////////

struct Triag2DLagrangeP1Fixture
{
  typedef std::vector<RealVector> NodesT;
  /// common setup for each test case
  Triag2DLagrangeP1Fixture() : mapped_coords(init_mapped_coords()), nodes(init_nodes())
  {
  }

  /// common tear-down for each test case
  ~Triag2DLagrangeP1Fixture()
  {
  }
  /// common values accessed by all tests goes here

  const CF::RealVector mapped_coords;
  const NodesT nodes;

  struct const_functor
  {
    const_functor(const NodesT& node_list) : m_nodes(node_list) {}
    template<typename GeoShapeF, typename SolShapeF>
    CF::Real valTimesDetJacobian(const CF::RealVector& mappedCoords)
    {
      return GeoShapeF::jacobian_determinant(mappedCoords, m_nodes);
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
    const CF::RealVector c2 = list_of(0.8)(2.1);
    return list_of(c0)(c1)(c2);
  }
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( Triag2DLagrangeP1Suite, Triag2DLagrangeP1Fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Volume )
{
  boost::multi_array<Real,2> nodes_triag2D (boost::extents[3][2]);
  nodes_triag2D[0][XX] = 0.0;     nodes_triag2D[0][YY] = 0.0;
  nodes_triag2D[1][XX] = 1.0;     nodes_triag2D[1][YY] = 0.0;
  nodes_triag2D[2][XX] = 1.0;     nodes_triag2D[2][YY] = 1.0;
  BOOST_CHECK_EQUAL(Triag2DLagrangeP1::volume(nodes_triag2D), 0.5);
}

BOOST_AUTO_TEST_CASE( Element )
{
  CArray::Ptr coordinates (new CArray("coordinates"));
  // Create a CElements component
  CElements::Ptr comp (new CElements("comp")) ;

  // The element is automatically triangle for now
  comp->initialize("Triag2DLagrangeP1",*coordinates);
  BOOST_CHECK_EQUAL(comp->element_type().shape(), GeoShape::TRIAG);
  BOOST_CHECK_EQUAL(comp->element_type().nb_nodes(), (Uint) 3);

  // Check volume calculation
  CArray::ArrayT coord(boost::extents[3][2]);
  coord[0][XX]=15; coord[0][YY]=15;
  coord[1][XX]=40; coord[1][YY]=25;
  coord[2][XX]=25; coord[2][YY]=30;
  std::vector<CArray::Row> coordvec;
  coordvec.reserve(3);
  coordvec.push_back(coord[0]);
  coordvec.push_back(coord[1]);
  coordvec.push_back(coord[2]);
  BOOST_CHECK_EQUAL(Triag2DLagrangeP1::volume(coordvec), 137.5);
}

BOOST_AUTO_TEST_CASE( ShapeFunction )
{
  const CF::RealVector reference_result = list_of(0.1)(0.1)(0.8);
  CF::RealVector result(3);
  Triag2DLagrangeP1::shape_function(mapped_coords, result);
  CF::Tools::Testing::Accumulator accumulator;
  CF::Tools::Testing::vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( MappedCoordinates )
{
  const CF::RealVector test_coords = list_of(0.8)(1.2);
  const CF::RealVector reference_result = list_of(1./3.)(1./3.);
  CF::RealVector result(2);
  Triag2DLagrangeP1::mapped_coordinates(test_coords, nodes, result);
  CF::Tools::Testing::Accumulator accumulator;
  CF::Tools::Testing::vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10); // Maximal difference can't be greater than 10 times the least representable unit
}

BOOST_AUTO_TEST_CASE( IntegrateConst )
{
  const_functor ftor(nodes);
  CF::Real result = 0.0;
  Gauss<Triag2DLagrangeP1>::integrateElement(ftor, result);
  BOOST_CHECK_LT(boost::accumulators::max(CF::Tools::Testing::test(result, Triag2DLagrangeP1::volume(nodes)).ulps), 1);
}

BOOST_AUTO_TEST_CASE( MappedGradient )
{
  CF::RealMatrix expected(Triag2DLagrangeP1::dimension, Triag2DLagrangeP1::nb_nodes);
  expected(0,0) = -1.;
  expected(1,0) = -1.;
  expected(0,1) = 1.;
  expected(1,1) = 0.;
  expected(0,2) = 0.;
  expected(1,2) = 1.;
  CF::RealMatrix result(Triag2DLagrangeP1::dimension, Triag2DLagrangeP1::nb_nodes);
  Triag2DLagrangeP1::mapped_gradient(mapped_coords, result);
  CF::Tools::Testing::Accumulator accumulator;
  CF::Tools::Testing::vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( JacobianDeterminant )
{
  // Shapefunction determinant should be double the volume for triangles
  BOOST_CHECK_LT(boost::accumulators::max(CF::Tools::Testing::test(0.5*Triag2DLagrangeP1::jacobian_determinant(mapped_coords, nodes), Triag2DLagrangeP1::volume(nodes)).ulps), 5);
}

BOOST_AUTO_TEST_CASE( Jacobian )
{
  CF::RealMatrix expected(2, 2);
  expected(0,0) = 0.6;
  expected(0,1) = 0.9;
  expected(1,0) = 0.3;
  expected(1,1) = 1.8;
  CF::RealMatrix result(2, 2);
  Triag2DLagrangeP1::jacobian(mapped_coords, nodes, result);
  CF::Tools::Testing::Accumulator accumulator;
  CF::Tools::Testing::vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

BOOST_AUTO_TEST_CASE( JacobianAdjoint )
{
  CF::RealMatrix expected(2, 2);
  expected(0,0) = 1.8;
  expected(0,1) = -0.9;
  expected(1,0) = -0.3;
  expected(1,1) = 0.6;
  CF::RealMatrix result(2, 2);
  Triag2DLagrangeP1::jacobian_adjoint(mapped_coords, nodes, result);
  CF::Tools::Testing::Accumulator accumulator;
  CF::Tools::Testing::vector_test(result, expected, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 2);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

