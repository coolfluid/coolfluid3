#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::LagrangeSF"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Mesh/Integrators/Gauss.hpp"
#include "Mesh/LagrangeSF/TriagP1.hpp"
#include "Mesh/P1/Triag2D.hpp"
#include "Tools/Difference/Difference.hpp"

using namespace boost::assign; // bring 'operator+=()' into scope
using namespace CF::Mesh::LagrangeSF;
using namespace CF::Mesh::Integrators;

//////////////////////////////////////////////////////////////////////////////

struct LagrangeSFTriagP1_Fixture
{
  /// common setup for each test case
  LagrangeSFTriagP1_Fixture() : mapped_coords(init_mapped_coords()), nodes(init_nodes())
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;

    //mapped_coords += 0.1, 0.8;
  }

  /// common tear-down for each test case
  ~LagrangeSFTriagP1_Fixture()
  {
  }
  /// common values accessed by all tests goes here

  const CF::RealVector mapped_coords;
  const NodesT nodes;

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
    static const CF::RealVector c0 = list_of(0.5)(0.3);
    static const CF::RealVector c1 = list_of(1.1)(1.2);
    static const CF::RealVector c2 = list_of(0.8)(2.1);
    return list_of(&c0)(&c1)(&c2);
  }

};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( LagrangeSFTriagP1, LagrangeSFTriagP1_Fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( computeShapeFunction )
{
  const CF::RealVector reference_result = list_of(0.1)(0.1)(0.8);
  CF::RealVector result(3);
  TriagP1::computeShapeFunction(mapped_coords, result);
  CF::Tools::Difference::Accumulator accumulator;
  CF::Tools::Difference::vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10);
}

BOOST_AUTO_TEST_CASE( computeJacobianDeterminant )
{
  CF::Mesh::P1::Triag2D triag_element; // todo: why is computeVolume not static?
  // Shapefunction determinant should be double the volume for triangles
  BOOST_CHECK_LT(boost::accumulators::max(CF::Tools::Difference::test(0.5*TriagP1::computeJacobianDeterminant(mapped_coords, nodes), triag_element.computeVolume(nodes)).ulps), 1);
}

BOOST_AUTO_TEST_CASE( integrateConst )
{
  CF::Mesh::P1::Triag2D triag_element;
  // Shapefunction determinant should be double the volume for triangles
  const_functor<TriagP1> ftor(nodes);
  CF::Real result = 0.0;
  Gauss<TriagP1>::integrate(ftor, result);
  BOOST_CHECK_LT(boost::accumulators::max(CF::Tools::Difference::test(result, triag_element.computeVolume(nodes)).ulps), 1);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

