// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/URI.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( CPath_TestSuite )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( constructors )
{
  // empty contructor
  URI p0;
  BOOST_CHECK(p0.empty());
  BOOST_CHECK_EQUAL(p0.string().size(), (size_t)0 );

  // string constructor
  URI p1 ( "lolo" );
  BOOST_CHECK(!p1.empty());
  BOOST_CHECK_EQUAL( std::strcmp( p1.string().c_str(), "lolo" ), 0 );

  // copy constructor
  URI p2 ( "koko" );
  URI p3 ( p2 );
  BOOST_CHECK(!p2.empty());
  BOOST_CHECK(!p3.empty());
  BOOST_CHECK_EQUAL( std::strcmp( p2.string().c_str(), p2.string().c_str() ), 0 );

  URI uri_absolute ( "cpath://hostname/root/component");
  URI uri_relative ( "../component");

  URI p4(uri_absolute);
  BOOST_CHECK(!p4.empty());
  BOOST_CHECK_EQUAL( p4.string(), "cpath://hostname/root/component");
  BOOST_CHECK(p4.is_absolute());

  URI p5(uri_relative);
  BOOST_CHECK(!p5.empty());
  BOOST_CHECK_EQUAL( p5.string(), "../component");
  BOOST_CHECK(p5.is_relative());

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( concatenation )
{
  URI p0 ( "/root/dir1" );
  URI p1 ( "dir2/dir3" );

  URI p2 = p0 / p1;
  BOOST_CHECK_EQUAL( std::strcmp( p2.string().c_str(), "/root/dir1/dir2/dir3" ), 0 );

  URI p3;
  p3 /= p0;
  BOOST_CHECK_EQUAL( std::strcmp( p3.string().c_str(), "/root/dir1" ), 0 );

  URI p5 = p0 / "dir5/dir55";
  BOOST_CHECK_EQUAL( std::strcmp( p5.string().c_str(), "/root/dir1/dir5/dir55" ), 0 );

  URI p6 = "/root/dir6";
  BOOST_CHECK_EQUAL( std::strcmp( p6.string().c_str(), "/root/dir6" ), 0 );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( protocol_management )
{
  URI uri("//Root/Component");

  // URI without any protocol
  BOOST_CHECK_EQUAL( uri.protocol(), URIProtocol::INVALID );
  BOOST_CHECK_EQUAL( uri.string(), std::string("//Root/Component") );
  BOOST_CHECK_EQUAL( uri.string_without_protocol(), std::string("//Root/Component") );

  // URI with a protocol
  URI uri2("cpath://Root/Component");
  BOOST_CHECK_EQUAL( uri2.protocol(), URIProtocol::CPATH );
  BOOST_CHECK_EQUAL( uri2.string(), std::string("cpath://Root/Component") );
  BOOST_CHECK_EQUAL( uri2.string_without_protocol(), std::string("//Root/Component") );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

