#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE CommonTest 

#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp> 

#include "Common/MyInt.hh"

using namespace boost;
using namespace COOLFluiD::Common;
  
BOOST_AUTO_TEST_SUITE(CommonTest)

BOOST_AUTO_TEST_CASE(test_getValue)
{
 MyInt i;
 MyInt i2(156);
 
 BOOST_CHECK_EQUAL(i.getValue(), 0);
 BOOST_CHECK_EQUAL(i2.getValue(), 156); 
}

BOOST_AUTO_TEST_SUITE_END()
