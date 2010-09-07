#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "Common/StringOps.hpp"
#include "Common/URI.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;

struct StringOps_Fixture
{
  /// common setup for each test case
  StringOps_Fixture()
  {
  }

  /// common tear-down for each test case
  ~StringOps_Fixture()
  {
  }

  /// possibly common functions used on the tests below

  /// common values accessed by all tests goes here

};

BOOST_FIXTURE_TEST_SUITE( StringOps_TestSuite, StringOps_Fixture )

BOOST_AUTO_TEST_CASE( boostFunctions )
{
  // the goal of the following tests is not to test the boost library
  // but to show some useful functions that were implemented in
  // CF 2.x.x and were removed in CF K3

  string parts[] = {"one", "two", "three"};
  string str = "  HeLLo World 123   ";
  string tmpStr;
  vector<string> myVector;

  // join
  BOOST_CHECK_EQUAL(algorithm::join(parts, " "), "one two three");

  // toLower
  tmpStr = str;
  algorithm::to_lower(tmpStr);
  BOOST_CHECK_EQUAL(tmpStr, "  hello world 123   ");

  // toUpper
  tmpStr = str;
  algorithm::to_upper(tmpStr);
  BOOST_CHECK_EQUAL(tmpStr, "  HELLO WORLD 123   ");

  // subst
  tmpStr = str;
  algorithm::replace_all(tmpStr, "World", "User");
  BOOST_CHECK_EQUAL(tmpStr, "  HeLLo User 123   ");

  // trimFront
  tmpStr = str;
  algorithm::trim_left(tmpStr);
  BOOST_CHECK_EQUAL(tmpStr, "HeLLo World 123   ");

  // trimRear
  tmpStr = str;
  algorithm::trim_right(tmpStr);
  BOOST_CHECK_EQUAL(tmpStr, "  HeLLo World 123");

  // trim
  tmpStr = str;
  algorithm::trim(tmpStr);
  BOOST_CHECK_EQUAL(tmpStr, "HeLLo World 123");

  // getWords
  algorithm::split(myVector, str, algorithm::is_any_of(" "), token_compress_on);
  BOOST_CHECK_EQUAL(myVector.size(), (unsigned int) 5);
  BOOST_CHECK_EQUAL(myVector[0], "");
  BOOST_CHECK_EQUAL(myVector[1], "HeLLo");
  BOOST_CHECK_EQUAL(myVector[2], "World");
  BOOST_CHECK_EQUAL(myVector[3], "123");
  BOOST_CHECK_EQUAL(myVector[4], "");

  // startsWith
  BOOST_CHECK(algorithm::starts_with(str, "  HeLLo"));

  // endsWith
  BOOST_CHECK(algorithm::ends_with(str, "World 123   "));

}

BOOST_AUTO_TEST_CASE( to_str )
{
  BOOST_CHECK_EQUAL(StringOps::to_str<int>(42), "42");
  BOOST_CHECK_EQUAL(StringOps::to_str<CF::Real>(3.14), "3.14");
}

BOOST_AUTO_TEST_CASE( from_str )
{
  BOOST_CHECK_EQUAL(StringOps::from_str<int>("42"), 42);
  BOOST_CHECK_EQUAL(StringOps::from_str<CF::Real>("3.14"), 3.14);
}

BOOST_AUTO_TEST_CASE( URI_test )
{
  Common::URI uri("file://hostname");
  uri /= "file_name.txt";
  BOOST_CHECK(uri.is_absolute());
  BOOST_CHECK_EQUAL(uri.string(),"file://hostname/file_name.txt");
  BOOST_CHECK_EQUAL(uri.base_path().string(),"file://hostname");
  
  Common::URI uri2("file_name.txt");
  BOOST_CHECK(uri2.is_relative());
  BOOST_CHECK_EQUAL(uri2.string(),"file_name.txt");
  
  Common::URI uri3("cpath://hostname");
  uri3 /= "component";
  BOOST_CHECK(uri3.is_absolute());
  BOOST_CHECK_EQUAL(uri3.string(),"cpath://hostname/component");
  BOOST_CHECK_EQUAL(uri3.base_path().string(),"cpath://hostname");
  
}
BOOST_AUTO_TEST_SUITE_END()
