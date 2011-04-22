// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for component factory"

#include <boost/test/unit_test.hpp>

#include "Common/CBuilder.hpp"
#include "Common/LibCommon.hpp"
#include "Common/CreateComponent.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;

//------------------------------------------------------------------------------------------

struct CBuilder_fixture
{
  /// common setup for each test case
  CBuilder_fixture() {}
  /// common tear-down for each test case
  ~CBuilder_fixture() {}
};

//------------------------------------------------------------------------------------------

class CAbstract : public Component {

public:

  typedef boost::shared_ptr<CAbstract> Ptr;
  typedef boost::shared_ptr<CAbstract const> ConstPtr;

public:

  CAbstract ( const std::string& name ) : Component(name) {}
  virtual ~CAbstract() {}
  static std::string type_name () { return "CAbstract"; }

};

class CConcrete1 : public CAbstract {

public:

  typedef boost::shared_ptr<CConcrete1> Ptr;
  typedef boost::shared_ptr<CConcrete1 const> ConstPtr;

public:

  CConcrete1 ( const std::string& name ) : CAbstract(name) {}
  virtual ~CConcrete1() {}
  static std::string type_name () { return "CConcrete1"; }

};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( CFactoryTest, CBuilder_fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( registration )
{
  Common::ComponentBuilder < CConcrete1 , CAbstract, Common::LibCommon > aBuilder;

  CAbstract::Ptr ptr = create_component_abstract_type< CAbstract >("CF.Common.CConcrete1","acomp");

  BOOST_CHECK( is_not_null(ptr) );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

