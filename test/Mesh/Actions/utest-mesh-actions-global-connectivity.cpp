// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests Mesh::Actions::CGlobalConnectivity"

#include <boost/test/unit_test.hpp>
#include <csignal>

#include "Common/Log.hpp"
#include "Common/CreateComponent.hpp"

#include "Mesh/Actions/CGlobalConnectivity.hpp"
#include "Mesh/Actions/CGlobalNumbering.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CMeshReader.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Mesh::Actions;

////////////////////////////////////////////////////////////////////////////////

struct TestCGlobalConnectivity_Fixture
{
  /// common setup for each test case
  TestCGlobalConnectivity_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~TestCGlobalConnectivity_Fixture()
  {
  }

  /// possibly common functions used on the tests below

  int m_argc;
  char** m_argv;

  /// common values accessed by all tests goes here
  static CMesh::Ptr mesh;
};

CMesh::Ptr TestCGlobalConnectivity_Fixture::mesh = allocate_component<CMesh>("mesh");

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( TestCGlobalConnectivity_TestSuite, TestCGlobalConnectivity_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Init )
{
  Core::instance().initiate(m_argc,m_argv);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( build )
{
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
  boost::filesystem::path fp_in("quadtriag.neu");
  meshreader->configure_property("read_boundaries",false);
  meshreader->read_from_to(fp_in,mesh);
  
  Core::instance().initiate(m_argc,m_argv);

  CGlobalNumbering::Ptr build_glb_numbering = allocate_component<CGlobalNumbering>("build_glb_numbering");
  build_glb_numbering->set_mesh(mesh);
  build_glb_numbering->configure_property("debug",true);
  build_glb_numbering->execute();
  
  CGlobalConnectivity::Ptr build_connectivity = allocate_component<CGlobalConnectivity>("build_glb_connectivity");
  build_connectivity->set_mesh(mesh);
  build_connectivity->execute();
  
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Terminate )
{
  Core::instance().terminate();
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

