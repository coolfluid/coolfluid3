#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include <iostream>

#include "Common/Log.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;

struct LogStreamFixture
{
  /// common setup for each test case
  LogStreamFixture() { m_stream = new LogStream("TestStream"); }

  /// common tear-down for each test case
  ~LogStreamFixture() { delete m_stream; }

  /// possibly common functions used on the tests below

  /// common values accessed by all tests goes here
  LogStream * m_stream;

};

/// @todo should this class stay here ?
struct MyStringForwarder : public LogStringForwarder
{
  void message(const std::string & str) { m_str = str; }

  std::string m_str;
};


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOST_FIXTURE_TEST_SUITE(LogStreamTestSuite,LogStreamFixture)

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOST_AUTO_TEST_CASE( setLogLevel )
{
  LogStreamFixture f;

  f.m_stream->setLogLevel(VERBOSE);
  f.m_stream->setLogLevel(LogStream::SCREEN, SILENT);

//  BOOST_CHECK_EQUAL( (int)f.m_stream->getLogLevel(LogStream::FILE), (int)VERBOSE);
  BOOST_CHECK_EQUAL( (int)f.m_stream->getLogLevel(LogStream::SCREEN), (int)SILENT);

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOST_AUTO_TEST_CASE( addStringForwarder )
{
  LogStreamFixture f;
  MyStringForwarder * forwarder = new MyStringForwarder();
  MyStringForwarder * anotherForwarder = new MyStringForwarder();

  // we have no forwarder
  BOOST_CHECK_EQUAL( f.m_stream->getStringForwarderCount(), (Uint) 0);

  // adding a forwarder should modify the count
  f.m_stream->addStringForwarder(forwarder);
  BOOST_CHECK_EQUAL( f.m_stream->getStringForwarderCount(), (Uint) 1);

  // adding a forwarder twice should be ignored
  f.m_stream->addStringForwarder(forwarder);
  BOOST_CHECK_EQUAL( f.m_stream->getStringForwarderCount(), (Uint) 1);

  f.m_stream->addStringForwarder(anotherForwarder);
  BOOST_CHECK_EQUAL( f.m_stream->getStringForwarderCount(), (Uint) 2);

  // adding a NULL forwarder should be ignored
  f.m_stream->addStringForwarder(NULL);
  BOOST_CHECK_EQUAL( f.m_stream->getStringForwarderCount(), (Uint) 2);

  // removing a forwarder should modify the count
  f.m_stream->removeStringForwarder(forwarder);
  BOOST_CHECK_EQUAL( f.m_stream->getStringForwarderCount(), (Uint) 1);

  // removing a forwarder twice should be ignored
  f.m_stream->removeStringForwarder(forwarder);
  BOOST_CHECK_EQUAL( f.m_stream->getStringForwarderCount(), (Uint) 1);

  // removing a NULL forwarder should be ignored
  f.m_stream->removeStringForwarder(NULL);
  BOOST_CHECK_EQUAL( f.m_stream->getStringForwarderCount(), (Uint) 1);

  delete forwarder;
  delete anotherForwarder;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOST_AUTO_TEST_CASE( useDestination )
{
  LogStreamFixture f;

  f.m_stream->useDestination(LogStream::SCREEN, true);
  f.m_stream->useDestination(LogStream::FILE, false);

  BOOST_CHECK(f.m_stream->isDestinationUsed(LogStream::SCREEN));
  BOOST_CHECK(!f.m_stream->isDestinationUsed(LogStream::FILE));
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOST_AUTO_TEST_CASE( setFilterRankZero )
{
  LogStreamFixture f;

  f.m_stream->setFilterRankZero(false);
  f.m_stream->setFilterRankZero(LogStream::SCREEN, true);

  BOOST_CHECK(!f.m_stream->getFilterRankZero(LogStream::FILE));
  BOOST_CHECK(!f.m_stream->getFilterRankZero(LogStream::STRING));
  BOOST_CHECK(!f.m_stream->getFilterRankZero(LogStream::SYNC_SCREEN));

  BOOST_CHECK(f.m_stream->getFilterRankZero(LogStream::SCREEN));
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOST_AUTO_TEST_CASE( setStamp )
{
LogStreamFixture f;

  f.m_stream->setStamp("");
  f.m_stream->setStamp(LogStream::SCREEN, "[%place%][%type%]");

  BOOST_CHECK_EQUAL(f.m_stream->getStamp(LogStream::FILE), "");
  BOOST_CHECK_EQUAL(f.m_stream->getStamp(LogStream::SCREEN), "[%place%][%type%]");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOST_AUTO_TEST_CASE( operators )
{
  /// test whether the string is forwarded
  LogStreamFixture f;
  MyStringForwarder * forwarder = new MyStringForwarder();

  f.m_stream->addStringForwarder(forwarder);
  // deactivate the stdout
  f.m_stream->useDestination(LogStream::SCREEN, false);

  *(f.m_stream) << "Hello world!";
  f.m_stream->flush();
  BOOST_CHECK_EQUAL(forwarder->m_str, std::string("Hello world!"));
  
  f.m_stream->setStamp(LogStream::STRING, "");
  // test the log levels redirection
  // for each level we do 4 checks: one for each level (explicitly given)
  // and a fourth where no level is given (meaning that the default has to 
  // be used)
  // 1. the stream log level is SILENT: nothing should be forwarded
  f.m_stream->setLogLevel(SILENT);
  forwarder->m_str.clear();
  *(f.m_stream) << "Hello world!";
  f.m_stream->flush();
  BOOST_CHECK_EQUAL(forwarder->m_str, std::string(""));
  
  forwarder->m_str.clear();
  *(f.m_stream) << SILENT << "Hello world!";
  f.m_stream->flush();
  BOOST_CHECK_EQUAL(forwarder->m_str, std::string(""));
  
  forwarder->m_str.clear();
  *(f.m_stream) << NORMAL << "Hello world!";
  f.m_stream->flush();
  BOOST_CHECK_EQUAL(forwarder->m_str, std::string(""));
  
  forwarder->m_str.clear();
  *(f.m_stream) << VERBOSE << "Hello world!";
  f.m_stream->flush();
  BOOST_CHECK_EQUAL(forwarder->m_str, std::string(""));
  
  // 2. the stream log level is NORMAL: only this level should be forwarded
  f.m_stream->setLogLevel(NORMAL);
  forwarder->m_str.clear();
  *(f.m_stream) << SILENT << "Hello world!";
  f.m_stream->flush();
  BOOST_CHECK_EQUAL(forwarder->m_str, std::string(""));
  
  forwarder->m_str.clear();
  *(f.m_stream) << "Hello world as NORMAL 1!";
  f.m_stream->flush();
  BOOST_CHECK_EQUAL(forwarder->m_str, std::string("Hello world as NORMAL 1!"));
  
  forwarder->m_str.clear();
  *(f.m_stream) << NORMAL << "Hello world as NORMAL 2!";
  f.m_stream->flush();
  BOOST_CHECK_EQUAL(forwarder->m_str, std::string("Hello world as NORMAL 2!"));
  
  forwarder->m_str.clear();
  *(f.m_stream) << VERBOSE << "Hello world!";
  f.m_stream->flush();
  BOOST_CHECK_EQUAL(forwarder->m_str, std::string(""));
  
  // 3. the stream log level is VERBOSE: NORMAL and VERBOSE should be forwarded
  f.m_stream->setLogLevel(VERBOSE);
  forwarder->m_str.clear();
  *(f.m_stream) << SILENT << "Hello world!";
  f.m_stream->flush();
  BOOST_CHECK_EQUAL(forwarder->m_str, std::string(""));
  
  forwarder->m_str.clear();
  *(f.m_stream) << NORMAL << "Hello world!";
  f.m_stream->flush();
  BOOST_CHECK_EQUAL(forwarder->m_str, std::string("Hello world!"));
  
  forwarder->m_str.clear();
  *(f.m_stream) << "Hello world as VERBOSE 1!";
  f.m_stream->flush();
  BOOST_CHECK_EQUAL(forwarder->m_str, std::string("Hello world as VERBOSE 1!"));
  
  forwarder->m_str.clear();
  *(f.m_stream) << VERBOSE << "Hello world as VERBOSE 2!";
  f.m_stream->flush();
  BOOST_CHECK_EQUAL(forwarder->m_str, std::string("Hello world as VERBOSE 2!"));
    
  /// Test the rank filter
  /// 1. with the filter disabled
  forwarder->m_str.clear();  
  f.m_stream->setFilterRankZero(LogStream::STRING, false);
  *(f.m_stream) << "Hello world!";
  f.m_stream->flush();
  BOOST_CHECK_EQUAL(forwarder->m_str, std::string("Hello world!"));

  /// 2. with the filter enabled
  forwarder->m_str.clear();  
  f.m_stream->setFilterRankZero(LogStream::STRING, true);
  *(f.m_stream) << "Hello world!";
  f.m_stream->flush();
  
  if(PE::getInstance().get_rank() == 0)
    BOOST_CHECK_EQUAL(forwarder->m_str, std::string("Hello world!"));
  else
    BOOST_CHECK_EQUAL(forwarder->m_str, std::string(""));
  
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOST_AUTO_TEST_SUITE_END()
