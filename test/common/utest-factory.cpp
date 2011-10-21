// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for component factory"

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/CRoot.hpp"

#include "test/common/DummyComponents.hpp"

#include "common/CFactories.hpp"
#include "common/CBuilder.hpp"


using namespace std;
using namespace boost;
using namespace cf3;
using namespace cf3::common;

//////////////////////////////////////////////////////////////////////////////

struct CFactoryFixture
{
  /// common setup for each test case
  CFactoryFixture() {}

  /// common tear-down for each test case
  ~CFactoryFixture() {}
};

//////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( CFactoryTest, CFactoryFixture )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( get_factory )
{
  CFactories::Ptr factories = Core::instance().root().get_child_ptr("Factories")->as_ptr< CFactories >();

  BOOST_CHECK( factories->get_factory< CAbstract >() != nullptr );

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( component_builder )
{
  ComponentBuilder< CConcrete1, CAbstract, LibCommon > cc1;
  ComponentBuilder< CConcrete2, CAbstract, LibCommon > cc2;

  CFactories::Ptr factories = Core::instance().root().get_child_ptr("Factories")->as_ptr< CFactories >();

  CFactoryT<CAbstract>::Ptr cabstract_factory = factories->get_factory< CAbstract >();
  BOOST_CHECK( cabstract_factory != nullptr );
  BOOST_CHECK_EQUAL( cabstract_factory->factory_type_name() , std::string("CAbstract") );

  CBuilder::Ptr cconcrete1_builder = cabstract_factory->get_child_ptr( "CF.Common.CConcrete1" )->as_ptr< CBuilder >();
  BOOST_CHECK( cconcrete1_builder != nullptr );
  BOOST_CHECK_EQUAL( cconcrete1_builder->builder_concrete_type_name() , std::string("CConcrete1") );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( LibraryName )
{
  const std::string builder_name1 = "CF.Mesh.Neu.CReader";
  BOOST_CHECK_EQUAL(CBuilder::extract_library_name(builder_name1), "coolfluid_mesh_neu");
  
  const std::string builder_name2 = "CF.UFEM.Test";
  BOOST_CHECK_EQUAL(CBuilder::extract_library_name(builder_name2), "coolfluid_ufem");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

