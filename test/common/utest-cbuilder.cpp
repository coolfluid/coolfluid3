// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for component factory"

#include <boost/test/unit_test.hpp>

#include "common/Core.hpp"
#include "common/Builder.hpp"
#include "common/LibCommon.hpp"
 

using namespace std;
using namespace boost;
using namespace cf3;
using namespace cf3::common;

//------------------------------------------------------------------------------------------

struct Builder_fixture
{
  /// common setup for each test case
  Builder_fixture() {}
  /// common tear-down for each test case
  ~Builder_fixture() {}
};

//------------------------------------------------------------------------------------------

class CAbstract : public Component
{
public:

  CAbstract ( const std::string& name ) : Component(name) {}
  virtual ~CAbstract() {}
  static std::string type_name () { return "CAbstract"; }

};

class CConcrete1 : public CAbstract
{
public:

  CConcrete1 ( const std::string& name ) : CAbstract(name) {}
  virtual ~CConcrete1() {}
  static std::string type_name () { return "CConcrete1"; }

};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( FactoryTest, Builder_fixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( registration )
{
  common::ComponentBuilder < CConcrete1 , CAbstract, common::LibCommon > aBuilder;

  boost::shared_ptr<CAbstract> ptr = build_component_abstract_type< CAbstract >("cf3.common.CConcrete1","acomp");

  BOOST_CHECK( is_not_null(ptr) );

  Core::instance().root().add_component(ptr);

  Handle<CConcrete1> ptr2 = Core::instance().root().create_component<CConcrete1>("cconc");

  BOOST_CHECK( ptr2 );

  Handle<CAbstract> cabs(ptr2);
  BOOST_CHECK( cabs );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

