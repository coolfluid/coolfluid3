// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh::actions::GlobalConnectivity"

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Core.hpp"
#include "common/List.hpp"

#include "common/PE/debug.hpp"
#include "common/PE/Comm.hpp"

#include "mesh/actions/GlobalConnectivity.hpp"
#include "mesh/actions/GlobalNumbering.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/MeshReader.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::mesh::actions;
using namespace cf3::common::PE;

////////////////////////////////////////////////////////////////////////////////

struct TestGlobalConnectivity_Fixture
{
  /// common setup for each test case
  TestGlobalConnectivity_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~TestGlobalConnectivity_Fixture()
  {
  }

  /// possibly common functions used on the tests below

  int m_argc;
  char** m_argv;

  /// common values accessed by all tests goes here
  static Handle< Mesh > mesh;
};

Handle< Mesh > TestGlobalConnectivity_Fixture::mesh = Core::instance().root().create_component<Mesh>("mesh");

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( TestGlobalConnectivity_TestSuite, TestGlobalConnectivity_Fixture )

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

  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");
  meshreader->options().configure_option("read_boundaries",false);
  meshreader->read_mesh_into("../../../resources/quadtriag.neu",*mesh);


  boost::shared_ptr<GlobalNumbering> build_glb_numbering = allocate_component<GlobalNumbering>("build_glb_numbering");
  build_glb_numbering->set_mesh(mesh);
  build_glb_numbering->options().configure_option("debug",true);
  build_glb_numbering->execute();

  boost::shared_ptr<GlobalConnectivity> build_connectivity = allocate_component<GlobalConnectivity>("build_glb_connectivity");
  build_connectivity->set_mesh(mesh);
  build_connectivity->execute();

  PEProcessSortedExecute(-1,
      std::cout << "rank = " << Comm::instance().rank() << std::endl;
      std::cout << "nodes = " << mesh->geometry_fields().glb_idx() << std::endl;
      std::cout << "ranks = " << mesh->geometry_fields().rank() << std::endl;
      boost_foreach(const Entities& entities, mesh->topology().elements_range())
      {
        std::cout << "elems = " << entities.glb_idx() << std::endl;
      }

  )
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

