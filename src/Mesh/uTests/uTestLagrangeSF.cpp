#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::LagrangeSF"

#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/CRoot.hpp"
#include "Mesh/LagrangeSF/TriagP1.hpp"
#include "Mesh/uTests/difference.hpp"

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( LagrangeSF )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( LagrangeSFTriagP1 )
{
  const CF::RealVector mapped_coords = boost::assign::list_of(0.1)(0.8);
  const CF::RealVector reference_result = boost::assign::list_of(0.1)(0.1)(0.8);
  CF::RealVector result(3);
  CF::Mesh::LagrangeSF::TriagP1::computeShapeFunction(mapped_coords, result);
  CF::difference::accumulator accumulator;
  CF::difference::vector_test(result, reference_result, accumulator);
  BOOST_CHECK_LT(boost::accumulators::max(accumulator.ulps), 10);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

