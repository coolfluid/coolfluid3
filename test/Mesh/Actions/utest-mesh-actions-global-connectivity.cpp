// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh::Actions::CGlobalConnectivity"

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/CRoot.hpp"
#include "common/PE/debug.hpp"
#include "common/PE/Comm.hpp"

#include "mesh/Actions/CGlobalConnectivity.hpp"
#include "mesh/Actions/CGlobalNumbering.hpp"
#include "mesh/CMeshTransformer.hpp"
#include "mesh/CMeshWriter.hpp"
#include "mesh/CMesh.hpp"
#include "mesh/CRegion.hpp"
#include "mesh/Geometry.hpp"
#include "mesh/CMeshReader.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::mesh::Actions;
using namespace cf3::common::PE;

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

CMesh::Ptr TestCGlobalConnectivity_Fixture::mesh = Core::instance().root().create_component_ptr<CMesh>("mesh");

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( TestCGlobalConnectivity_TestSuite, TestCGlobalConnectivity_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Init )
{
  Core::instance().initiate(m_argc,m_argv);
  PE::Comm::instance().init(m_argc,m_argv);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( build )
{
  Core::instance().initiate(m_argc,m_argv);

  CMeshReader::Ptr meshreader = build_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
  meshreader->configure_option("read_boundaries",false);
  meshreader->read_mesh_into("quadtriag.neu",*mesh);


  CGlobalNumbering::Ptr build_glb_numbering = allocate_component<CGlobalNumbering>("build_glb_numbering");
  build_glb_numbering->set_mesh(mesh);
  build_glb_numbering->configure_option("debug",true);
  build_glb_numbering->execute();

  CGlobalConnectivity::Ptr build_connectivity = allocate_component<CGlobalConnectivity>("build_glb_connectivity");
  build_connectivity->set_mesh(mesh);
  build_connectivity->execute();

  PEProcessSortedExecute(-1,
      std::cout << "rank = " << Comm::instance().rank() << std::endl;
      std::cout << "nodes = " << mesh->geometry().glb_idx() << std::endl;
      std::cout << "ranks = " << mesh->geometry().rank() << std::endl;
      boost_foreach(const CEntities& entities, mesh->topology().elements_range())
      {
        std::cout << "elems = " << entities.glb_idx() << std::endl;
      }

  )

      RealVector coord1(2); coord1 << 2 , 8;
      RealVector coord2(2); coord2 << 3 , 11;

      std::size_t hash1 = CGlobalNumbering::hash_value(coord1);
      std::size_t hash2 = CGlobalNumbering::hash_value(coord2);
      BOOST_CHECK(hash1 != hash2);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Terminate )
{
  PE::Comm::instance().finalize();
  Core::instance().terminate();
}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

