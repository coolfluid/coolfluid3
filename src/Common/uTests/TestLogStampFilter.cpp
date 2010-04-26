#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

#include "Common/LogStampFilter.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;

struct LogStampFilterFixture
{
  /// common setup for each test case
  LogStampFilterFixture() :
  m_buffer(),
  m_sink(iostreams::back_inserter(m_buffer))
  { 
    m_filter = new LogStampFilter("TestStream"); 
  }

  /// common tear-down for each test case
  ~LogStampFilterFixture() { delete m_filter; }

  /// possibly common functions used on the tests below

  /// common values accessed by all tests goes here
  LogStampFilter * m_filter;
  
  string m_buffer;
  
  iostreams::back_insert_device<string> m_sink;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOST_FIXTURE_TEST_SUITE(LogStampFilterTestSuite,LogStampFilterFixture)

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOST_AUTO_TEST_CASE( write )
{
  /// test whether the string is forwarded
  LogStampFilterFixture f;
  string str = "Hello world!";
  unsigned int writtenBytes;
  
  writtenBytes = f.m_filter->write(f.m_sink, str.c_str(), str.length());
  f.m_filter->endMessage();
  BOOST_CHECK_EQUAL(str, f.m_buffer);
  BOOST_CHECK_EQUAL(writtenBytes, f.m_buffer.length());
  
  f.m_buffer.clear();
  
  // test the stamps ("TestStream" is the name of the filter created by
  // LogStampFilterFixture class)
  f.m_filter->setStamp("<%type%> ");
  writtenBytes = f.m_filter->write(f.m_sink, str.c_str(), str.length());
  f.m_filter->endMessage(); 
  BOOST_CHECK_EQUAL(string("<TestStream> ") + str, f.m_buffer);
  // below : +13 because of the stamp size
  BOOST_CHECK_EQUAL(writtenBytes, str.length() + 13); 
  
  f.m_buffer.clear();

  // test the stamps ("TestStream" is the name of the filter created by
  // LogStampFilterFixture class)
  f.m_filter->setStamp("<%tpye%> ");
  writtenBytes = f.m_filter->write(f.m_sink, str.c_str(), str.length());
  f.m_filter->endMessage(); 
  BOOST_CHECK_EQUAL(string("<%tpye%> ") + str, f.m_buffer);
  // below : +9 because of the stamp size
  BOOST_CHECK_EQUAL(writtenBytes, str.length() + 9); 
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOST_AUTO_TEST_CASE( setPlace )
{
  LogStampFilterFixture f;
  CodeLocation cl = FromHere();
  string str = "Hello world!";
  unsigned int writtenBytes;
  
  f.m_filter->setStamp("--%place%-- ");
  f.m_filter->setPlace(cl);
  writtenBytes = f.m_filter->write(f.m_sink, str.c_str(), str.length());
  f.m_filter->endMessage();
  BOOST_CHECK_EQUAL(string("--") + cl.short_str() + string("-- ") + str, f.m_buffer);
  BOOST_CHECK_EQUAL(writtenBytes, str.length() + cl.short_str().length() + 5); 
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOST_AUTO_TEST_SUITE_END()
