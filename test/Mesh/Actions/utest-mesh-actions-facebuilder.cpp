// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests Mesh::Actions::CBuildFaces"

#include <boost/test/unit_test.hpp>
#include "Common/Log.hpp"
#include "Common/CreateComponent.hpp"

#include "Mesh/Actions/CBuildFaces.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Mesh::Actions;

////////////////////////////////////////////////////////////////////////////////

struct TestCBuildFaces_Fixture
{
  /// common setup for each test case
  TestCBuildFaces_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~TestCBuildFaces_Fixture()
  {
  }

  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( TestCBuildFaces_TestSuite, TestCBuildFaces_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Constructors)
{
  CBuildFaces::Ptr facebuilder = allocate_component<CBuildFaces>("facebuilder");
  BOOST_CHECK_EQUAL(facebuilder->name(),"facebuilder");
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( make_interfaces )
{
  
  CMesh::Ptr mesh = allocate_component<CMesh>("mesh");
  CRegion& R1 = mesh->create_region("R1");
  CRegion& R2 = mesh->create_region("R2");
  CRegion& R3 = mesh->create_region("R3");
  CRegion& R11 = R1.create_region("R1");
  CRegion& R12 = R1.create_region("R2");
  CRegion& R13 = R1.create_region("R3");
  CRegion& R14 = R1.create_region("R4");
  CRegion& R21 = R2.create_region("R1");

  CBuildFaces::Ptr facebuilder = allocate_component<CBuildFaces>("facebuilder");
  
  std::vector<std::string> args;
  facebuilder->transform(mesh,args);
  
  CFinfo << mesh->tree() << CFendl;
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

