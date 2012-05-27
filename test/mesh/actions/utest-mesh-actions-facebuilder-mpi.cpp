// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh::actions::BuildFaces"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>

#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "common/OptionList.hpp"

#include "common/FindComponents.hpp"
#include "common/PE/Comm.hpp"
#include "common/PE/debug.hpp"

#include "mesh/actions/BuildFaces.hpp"
#include "mesh/actions/BuildFaceNormals.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Faces.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/Field.hpp"
#include "mesh/FaceCellConnectivity.hpp"
#include "mesh/Cells.hpp"
#include "mesh/SimpleMeshGenerator.hpp"

using namespace cf3;
using namespace boost::assign;
using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::mesh::actions;

////////////////////////////////////////////////////////////////////////////////

struct TestBuildFaces_Fixture
{
  /// common setup for each test case
  TestBuildFaces_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~TestBuildFaces_Fixture()
  {
  }

  /// possibly common functions used on the tests below
  int m_argc;
  char** m_argv;


  /// common values accessed by all tests goes here
  static Handle<Mesh> mesh;
};

Handle<Mesh> TestBuildFaces_Fixture::mesh = Core::instance().root().create_component<Mesh>("mesh");

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( TestBuildFaces_TestSuite, TestBuildFaces_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi)
{
  Core::instance().initiate(m_argc,m_argv);
  PE::Comm::instance().init(m_argc,m_argv);
  Core::instance().environment().options().set("log_level",(Uint)DEBUG);
  Core::instance().environment().options().set("only_cpu0_writes",false);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_mesh )
{
  boost::shared_ptr<MeshReader> meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");
  meshreader->read_mesh_into("../../../resources/quadtriag.neu",*mesh);

//  PEProcessSortedExecute(-1,
//  std::cout << PERank << mesh->tree() << std::endl<<std::endl;
//                         )


}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( build_faces )
{
  boost::shared_ptr<BuildFaces> facebuilder = allocate_component<BuildFaces>("facebuilder");

  facebuilder->set_mesh(mesh);
  facebuilder->execute();

  PEProcessSortedExecute(-1,
                         std::cout << "checking rank " << PERank << std::endl;
  BOOST_CHECK( is_not_null(Core::instance().root().access_component("mesh/topology/inlet")) );
  BOOST_CHECK( is_not_null(Core::instance().root().access_component("mesh/topology/outlet")) );
  BOOST_CHECK( is_not_null(Core::instance().root().access_component("mesh/topology/wall")) );
  BOOST_CHECK( is_not_null(Core::instance().root().access_component("mesh/topology/liquid")) );
  BOOST_CHECK( is_not_null(Core::instance().root().access_component("mesh/topology/liquid/cells")) );
  BOOST_CHECK( is_not_null(Core::instance().root().access_component("mesh/topology/liquid/inner_faces")) );
//  BOOST_CHECK( is_not_null(Core::instance().root().access_component("mesh/topology/liquid/outer_faces")) );
  BOOST_CHECK( is_not_null(Core::instance().root().access_component("mesh/topology/gas")) );
  BOOST_CHECK( is_not_null(Core::instance().root().access_component("mesh/topology/gas/cells")) );
  BOOST_CHECK( is_not_null(Core::instance().root().access_component("mesh/topology/gas/inner_faces")) );
//  BOOST_CHECK( is_not_null(Core::instance().root().access_component("mesh/topology/gas/outer_faces")) );
//  std::cout << mesh->topology().access_component("quadtriag/gas").tree() << std::endl;
  );

}

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( write_mesh )
{

//  PEProcessSortedExecute(-1,
//  std::cout << PERank << mesh->tree() << std::endl<<std::endl;
//                         )

  boost::shared_ptr<MeshWriter> tecwriter = build_component_abstract_type<MeshWriter>("cf3.mesh.tecplot.Writer","meshwriter");
  tecwriter->write_from_to(*mesh,URI("file:quadtriag-faces.plt"));

  boost::shared_ptr<MeshWriter> gmshwriter = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  gmshwriter->write_from_to(*mesh,URI("file:quadtriag-faces.msh"));

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
  Core::instance().terminate();
  PE::Comm::instance().finalize();
}

//////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

