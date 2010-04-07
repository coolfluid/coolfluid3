#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE LogStreamTest 

#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp> 

//#include "Common/Log.hh"

using namespace boost;
//using namespace CF;
//using namespace CF::Common;
  
BOOST_AUTO_TEST_SUITE(LogStreamTest)

BOOST_AUTO_TEST_CASE(test_setLogLevel)
{
  // LogStream stream("TestStream");
 
// stream.setLogLevel(VERBOSE);
// stream.setLogLevel(LogStream::SCREEN, SILENT);
 
// BOOST_CHECK_EQUAL(stream.getLogLevel(LogStream::FILE), VERBOSE);
// BOOST_CHECK_EQUAL(stream.getLogLevel(LogStream::FILE), SILENT);

// MyInt i;
// MyInt i2(156);
// 
// BOOST_CHECK_EQUAL(i.getValue(), 0);
// BOOST_CHECK_EQUAL(i2.getValue(), 156); 
}

BOOST_AUTO_TEST_SUITE_END()
