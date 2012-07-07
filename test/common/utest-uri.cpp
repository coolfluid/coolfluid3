// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for URIs manipulation"

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/URI.hpp"

using namespace std;
using namespace boost;
using namespace cf3;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( URI_TestSuite )

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
  BOOST_CHECK_EQUAL( std::strcmp( p1.path().c_str(), "lolo" ), 0 );
  BOOST_CHECK_EQUAL( std::strcmp( p1.string().c_str(), "cpath:lolo" ), 0 );

  // copy constructor
  URI p2 ( "koko" );
  URI p3 ( p2 );
  BOOST_CHECK(!p2.empty());
  BOOST_CHECK(!p3.empty());
  BOOST_CHECK_EQUAL( std::strcmp( p2.string().c_str(), p3.string().c_str() ), 0 );

  URI uri_absolute ( "cpath:/hostname/root/component");
  URI uri_relative ( "../component");

  URI p4(uri_absolute);
  BOOST_CHECK(!p4.empty());
  BOOST_CHECK_EQUAL( p4.string(), "cpath:/hostname/root/component");
  BOOST_CHECK(p4.is_absolute());

  URI p5(uri_relative);
  BOOST_CHECK(!p5.empty());
  BOOST_CHECK_EQUAL( p5.string(), "cpath:../component");
  BOOST_CHECK(p5.is_relative());

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( concatenation )
{
  URI p0 ( "/root/dir1" );
  URI p1 ( "dir2/dir3" );

  URI p2 = p0 / p1;
  BOOST_CHECK_EQUAL( std::strcmp( p2.path().c_str(), "/root/dir1/dir2/dir3" ), 0 );

  URI p3;
  p3 /= p0;
  BOOST_CHECK_EQUAL( std::strcmp( p3.path().c_str(), "/root/dir1" ), 0 );

  URI p5 = p0 / "dir5/dir55";
  BOOST_CHECK_EQUAL( std::strcmp( p5.path().c_str(), "/root/dir1/dir5/dir55" ), 0 );

  URI p6 = "/root/dir6";
  BOOST_CHECK_EQUAL( std::strcmp( p6.path().c_str(), "/root/dir6" ), 0 );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( protocol_management )
{
  URI uri("//Component");

  // URI without any protocol
  BOOST_CHECK_EQUAL( uri.scheme(), URI::Scheme::CPATH );
  BOOST_CHECK_EQUAL( uri.string(), std::string("cpath:/Component") );
  BOOST_CHECK_EQUAL( uri.path(), std::string("/Component") );

  // URI with a cpath
  URI uri2("cpath:/Component");
  BOOST_CHECK_EQUAL( uri2.scheme(), URI::Scheme::CPATH );
  BOOST_CHECK_EQUAL( uri2.string(), std::string("cpath:/Component") );
  BOOST_CHECK_EQUAL( uri2.path(), std::string("/Component") );

  // URI with a file
  URI uri3("file:///etc/fstab.txt");
  BOOST_CHECK_EQUAL( uri3.scheme(), URI::Scheme::FILE );
  BOOST_CHECK_EQUAL( uri3.string(), std::string("file:///etc/fstab.txt") );
  BOOST_CHECK_EQUAL( uri3.path(), std::string("///etc/fstab.txt") );
  BOOST_CHECK_EQUAL( uri3.base_path().string(), std::string("file:///etc") );
  BOOST_CHECK_EQUAL( uri3.extension(), std::string(".txt") );
  BOOST_CHECK_EQUAL( uri3.base_name(), std::string("fstab") );


  // URI with an http address
  
  URI uri4("http://coolfluidsrv.vki.ac.be");
  BOOST_CHECK(true);
  BOOST_CHECK_EQUAL( uri4.scheme(), URI::Scheme::HTTP );
  BOOST_CHECK_EQUAL( uri4.string(), std::string("http://coolfluidsrv.vki.ac.be") );
  BOOST_CHECK_EQUAL( uri4.path(), std::string("//coolfluidsrv.vki.ac.be") );

  // URI with an https address
  URI uri5("https://coolfluidsrv.vki.ac.be");
  BOOST_CHECK_EQUAL( uri5.scheme(), URI::Scheme::HTTPS );
  BOOST_CHECK_EQUAL( uri5.string(), std::string("https://coolfluidsrv.vki.ac.be") );
  BOOST_CHECK_EQUAL( uri5.path(), std::string("//coolfluidsrv.vki.ac.be") );

  // URI with a very long http address
  URI uri6("http://coolfluidsrv.vki.ac.be/redmine/projects/activity/coolfluid3?"
           "show_issues=1&show_changesets=1&show_news=1&show_documents=1&"
           "show_files=1&show_wiki_edits=1");
  BOOST_CHECK_EQUAL( uri6.scheme(), URI::Scheme::HTTP );
  BOOST_CHECK_EQUAL( uri6.string(),
                     std::string("http://coolfluidsrv.vki.ac.be/redmine/projects/activity/"
                                 "coolfluid3?show_issues=1&show_changesets=1&show_news=1&"
                                 "show_documents=1&show_files=1&show_wiki_edits=1") );
  BOOST_CHECK_EQUAL( uri6.path(),
                     std::string("//coolfluidsrv.vki.ac.be/redmine/projects/activity/"
                                 "coolfluid3?show_issues=1&show_changesets=1&show_news=1&"
                                 "show_documents=1&show_files=1&show_wiki_edits=1") );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( memory_failure )
{
  BOOST_CHECK_EQUAL( URI::Scheme::Convert::instance().to_str(URI::Scheme::CPATH), std::string("cpath") );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

