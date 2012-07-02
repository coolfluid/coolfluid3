// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::mesh::gmsh::Reader"

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/OptionList.hpp"


#include "common/Core.hpp"
#include "common/Environment.hpp"

#include "math/VariablesDescriptor.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/Field.hpp"
#include "mesh/Entities.hpp"
#include "mesh/Space.hpp"
#include "common/DynTable.hpp"
#include "common/List.hpp"
#include "common/Table.hpp"
#include "mesh/Dictionary.hpp"

using namespace std;
using namespace boost;
using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

struct gmshReaderMPITests_Fixture
{
  /// common setup for each test case
  gmshReaderMPITests_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~gmshReaderMPITests_Fixture()
  {
  }
  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here
  int    m_argc;
  char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( gmshReaderMPITests_TestSuite, gmshReaderMPITests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
  Core::instance().initiate(m_argc,m_argv);
  Core::instance().environment().options().set("log_level",4u);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_triag_p1 )
{

  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.gmsh.Reader","meshreader");
  BOOST_CHECK_EQUAL( meshreader->name() , "meshreader" );
  BOOST_CHECK_EQUAL( meshreader->get_format() , "Gmsh" );
  std::vector<std::string> extensions = meshreader->get_extensions();
  BOOST_CHECK_EQUAL( extensions[0] , ".msh" );

//	meshreader->options().set("Repartition",true);
//	meshreader->options().set("OutputRank",(Uint) 0);
//	meshreader->options().set("Unified Zones",false);


  // the mesh to store in
  Mesh& mesh = *Core::instance().root().create_component<Mesh>("mesh_2d_triag_p1");

  // CFinfo.setFilterRankZero(false);
  meshreader->read_mesh_into("../../resources/rectangle-tg-p1.msh",mesh);
  // CFinfo.setFilterRankZero(true);

  // CFinfo << mesh.tree() << CFendl;

boost::shared_ptr< MeshWriter > mesh_writer =
    build_component_abstract_type<MeshWriter> ("cf3.mesh.gmsh.Writer", "GmshWriter" );
mesh_writer->options().set("mesh",mesh.handle<Mesh const>());
mesh_writer->options().set("file",URI("rectangle-tg-p1-out.msh"));
mesh_writer->execute();

  CFinfo << "elements count = " << find_component<Region>(mesh).recursive_elements_count(true) << CFendl;
  CFinfo << "nodes count    = " << find_component<Region>(mesh).recursive_nodes_count() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_triag_p2 )
{

  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.gmsh.Reader","meshreader");

  // the mesh to store in
  Mesh& mesh = *Core::instance().root().create_component<Mesh>("mesh_2d_triag_p2");

  // CFinfo.setFilterRankZero(false);
  meshreader->read_mesh_into("../../resources/rectangle-tg-p2.msh",mesh);
  // CFinfo.setFilterRankZero(true);

  // CFinfo << mesh.tree() << CFendl;

  boost::shared_ptr< MeshWriter > mesh_writer =
    build_component_abstract_type<MeshWriter> ("cf3.mesh.gmsh.Writer", "GmshWriter" );
  mesh_writer->options().set("mesh",mesh.handle<Mesh const>());
  mesh_writer->options().set("file",URI("rectangle-tg-p2-out.msh"));
  mesh_writer->execute();

  CFinfo << "elements count = " << find_component<Region>(mesh).recursive_elements_count(true) << CFendl;
  CFinfo << "nodes count    = " << find_component<Region>(mesh).recursive_nodes_count() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_quad_p1 )
{

  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.gmsh.Reader","meshreader");

  // the mesh to store in
  Mesh& mesh = *Core::instance().root().create_component<Mesh>("mesh_2d_quad_p1");

  // CFinfo.setFilterRankZero(false);
  meshreader->read_mesh_into("../../resources/rectangle-qd-p2.msh",mesh);
  // CFinfo.setFilterRankZero(true);

  // CFinfo << mesh.tree() << CFendl;

  boost::shared_ptr< MeshWriter > mesh_writer =
    build_component_abstract_type<MeshWriter> ("cf3.mesh.gmsh.Writer", "GmshWriter" );
  mesh_writer->options().set("mesh",mesh.handle<Mesh const>());
  mesh_writer->options().set("file",URI("rectangle-qd-p1-out.msh"));
  mesh_writer->execute();

  CFinfo << "elements count = " << find_component<Region>(mesh).recursive_elements_count(true) << CFendl;
  CFinfo << "nodes count    = " << find_component<Region>(mesh).recursive_nodes_count() << CFendl;

}
////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_quad_p2 )
{

  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.gmsh.Reader","meshreader");

  // the mesh to store in
  Mesh& mesh = *Core::instance().root().create_component<Mesh>("mesh_2d_quad_p2");

  // CFinfo.setFilterRankZero(false);
  meshreader->read_mesh_into("../../resources/rectangle-qd-p2.msh",mesh);
  // CFinfo.setFilterRankZero(true);

  // CFinfo << mesh.tree() << CFendl;

  boost::shared_ptr< MeshWriter > mesh_writer =
    build_component_abstract_type<MeshWriter> ("cf3.mesh.gmsh.Writer", "GmshWriter" );
  mesh_writer->options().set("mesh",mesh.handle<Mesh const>());
  mesh_writer->options().set("file",URI("rectangle-qd-p2-out.msh"));
  mesh_writer->execute();

  CFinfo << "elements count = " << find_component<Region>(mesh).recursive_elements_count(true) << CFendl;
  CFinfo << "nodes count    = " << find_component<Region>(mesh).recursive_nodes_count() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_mix_p1 )
{

  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.gmsh.Reader","meshreader");

  // the mesh to store in
  Mesh& mesh = *Core::instance().root().create_component<Mesh>("mesh_2d_mix_p1");

  // CFinfo.setFilterRankZero(false);
  meshreader->read_mesh_into("../../resources/rectangle-mix-p1.msh",mesh);
  // CFinfo.setFilterRankZero(true);

  // CFinfo << mesh.tree() << CFendl;

  boost::shared_ptr< MeshWriter > mesh_writer =
    build_component_abstract_type<MeshWriter> ("cf3.mesh.gmsh.Writer", "GmshWriter" );
  mesh_writer->options().set("mesh",mesh.handle<Mesh const>());
  mesh_writer->options().set("file",URI("rectangle-mix-p1-out.msh"));
  mesh_writer->execute();

  CFinfo << "elements count = " << find_component<Region>(mesh).recursive_elements_count(true) << CFendl;
  CFinfo << "nodes count    = " << find_component<Region>(mesh).recursive_nodes_count() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_mix_p2 )
{

  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.gmsh.Reader","meshreader");

  // the mesh to store in
  Mesh& mesh = *Core::instance().root().create_component<Mesh>("mesh_2d_mix_p2");

  // CFinfo.setFilterRankZero(false);
  meshreader->read_mesh_into("../../resources/rectangle-mix-p2.msh",mesh);
  // CFinfo.setFilterRankZero(true);

  // CFinfo << mesh.tree() << CFendl;

  boost::shared_ptr< MeshWriter > mesh_writer =
    build_component_abstract_type<MeshWriter> ("cf3.mesh.gmsh.Writer", "GmshWriter" );
  mesh_writer->options().set("mesh",mesh.handle<Mesh const>());
  mesh_writer->options().set("file",URI("rectangle-mix-p2-out.msh"));
  mesh_writer->execute();

  CFinfo << "elements count = " << find_component<Region>(mesh).recursive_elements_count(true) << CFendl;
  CFinfo << "nodes count    = " << find_component<Region>(mesh).recursive_nodes_count() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_mix_p1_out )
{
  BOOST_CHECK(true);
  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.gmsh.Reader","meshreader");

  // the mesh to store in
  Mesh& mesh = *Core::instance().root().create_component<Mesh>("mesh_2d_mix_p1_out");

  // CFinfo.setFilterRankZero(false);
  meshreader->read_mesh_into("rectangle-mix-p1-out_P0.msh",mesh);
  // CFinfo.setFilterRankZero(true);
  BOOST_CHECK(true);

  // CFinfo << mesh.tree() << CFendl;

  Field& nodal = mesh.geometry_fields().create_field("nodal" , "nodal[vector]");
  nodal.descriptor().options().set(common::Tags::dimension(),mesh.dimension());
  for (Uint n=0; n<nodal.size(); ++n)
  {
    for(Uint j=0; j<nodal.row_size(); ++j)
      nodal[n][j] = n;
  }

  mesh.create_discontinuous_space("elems_P0","cf3.mesh.LagrangeP0");

  Field& cell_centred = mesh.geometry_fields().create_field("cell_centred","cell_centred[vector]");
  for (Uint e=0; e<cell_centred.size(); ++e)
  {
    for(Uint j=0; j<cell_centred.row_size(); ++j)
      cell_centred[e][j] = e;
  }

  std::vector<URI> fields;
  fields.push_back(cell_centred.uri());
  fields.push_back(nodal.uri());

  std::vector<URI> regions;
  regions.push_back(mesh.uri()/"topology/inlet");
  regions.push_back(mesh.uri()/"topology/outlet");
  regions.push_back(mesh.uri()/"topology/wall");
  regions.push_back(mesh.uri()/"topology/left");

  BOOST_CHECK(true);

  boost::shared_ptr< MeshWriter > mesh_writer =
    build_component_abstract_type<MeshWriter> ("cf3.mesh.gmsh.Writer", "GmshWriter" );

  BOOST_CHECK(true);

  Handle<Mesh> mesh_arg = mesh.handle<Mesh>();
  BOOST_CHECK(true);
  mesh_writer->options().set("mesh",mesh_arg);
  mesh_writer->options().set("fields",fields);
  mesh_writer->options().set("file",URI("rectangle-mix-p1-out-out.msh"));
  mesh_writer->options().set("regions",regions);
  mesh_writer->execute();

  BOOST_CHECK(true);

  CFinfo << "elements count = " << find_component<Region>(mesh).recursive_elements_count(true) << CFendl;
  CFinfo << "nodes count    = " << find_component<Region>(mesh).recursive_nodes_count() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
  Core::instance().terminate();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

