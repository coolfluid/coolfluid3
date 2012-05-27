
// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh reading"

#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>

#include "common/OptionT.hpp"
#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Core.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionURI.hpp"
#include "common/Group.hpp"
#include "common/FindComponents.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/FileOperations.hpp"
#include "common/XML/SignalFrame.hpp"
#include "common/XML/SignalOptions.hpp"

#include "mesh/Domain.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "common/Table.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/MeshTransformer.hpp"

using namespace std;
using namespace boost;
using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::common;
using namespace cf3::common::XML;

////////////////////////////////////////////////////////////////////////////////

struct MeshReading_Fixture
{
  /// common setup for each test case
  MeshReading_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;

    root = allocate_component<Group>("Root");
    reader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","MyReader");
    domain = root->create_component<Domain>("MyDom");

    root->add_component( reader );

    ExceptionManager::instance().ExceptionOutputs = false;
    ExceptionManager::instance().ExceptionDumps = false;
  }

  /// common tear-down for each test case
  ~MeshReading_Fixture()
  {
  }

  /// possibly common functions used on the tests below
  boost::shared_ptr< Component > root;
  boost::shared_ptr< MeshReader > reader;
  Handle< Domain > domain;

  /// common values accessed by all tests goes here
  int    m_argc;
  char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( MeshReading_TestSuite, MeshReading_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Constructors )
{
  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");
  BOOST_CHECK_EQUAL(meshreader->name(),"meshreader");
  BOOST_CHECK_EQUAL(meshreader->get_format(),"neu");

  boost::shared_ptr< MeshWriter > meshwriter = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  BOOST_CHECK_EQUAL(meshwriter->name(),"meshwriter");
  BOOST_CHECK_EQUAL(meshwriter->get_format(),"Gmsh");

  boost::shared_ptr< MeshWriter > neu_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.neu.Writer","meshwriter");
  BOOST_CHECK_EQUAL(neu_writer->name(),"meshwriter");
  BOOST_CHECK_EQUAL(neu_writer->get_format(),"neu");

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( quadtriag_readneu_writeGmsh_writeneu )
{
  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");

  // the mesh to store in
  Mesh& mesh = *Core::instance().root().create_component<Mesh>  ( "quadtriag" );

  meshreader->read_mesh_into("../../resources/quadtriag.neu",mesh);
  BOOST_CHECK(true);
  boost::shared_ptr< MeshWriter > gmsh_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  BOOST_CHECK_NO_THROW(gmsh_writer->write_from_to(mesh,"quadtriag.msh"));

  boost::shared_ptr< MeshWriter > neu_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.neu.Writer","meshwriter");
  BOOST_CHECK_NO_THROW(neu_writer->write_from_to(mesh,"quadtriag_write.neu"));

  BOOST_CHECK_EQUAL(mesh.topology().recursive_nodes_count(), (Uint) 16);
  BOOST_CHECK_EQUAL(mesh.topology().recursive_elements_count(true), (Uint) 28);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( quadtriag_read_Newneu_writeGmsh )
{
  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");
  boost::shared_ptr< MeshWriter > meshwriter = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");

  // the mesh to store in
  Mesh& mesh = *Core::instance().root().create_component<Mesh>  ( "quadtriag_write" );

  //CFinfo << "ready to read" << CFendl;
  BOOST_CHECK_NO_THROW(meshreader->read_mesh_into("quadtriag_write.neu",mesh));

  //CFinfo << "ready to write" << CFendl;
  BOOST_CHECK_NO_THROW(meshwriter->write_from_to(mesh,"quadtriag_write.msh"));
  BOOST_CHECK_EQUAL(mesh.topology().recursive_nodes_count(), (Uint) 16);
  BOOST_CHECK_EQUAL(mesh.topology().recursive_elements_count(true), (Uint) 28);

//  boost::shared_ptr< MeshTransformer > meshinfo = build_component_abstract_type<MeshTransformer>("Info","meshinfo");
//  meshinfo->transform(mesh);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( hextet_readneu_writeGmsh_writeneu )
{
  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");

  // the mesh to store in
  Mesh& mesh = *Core::instance().root().create_component<Mesh>  ( "hextet" );

  BOOST_CHECK_NO_THROW(meshreader->read_mesh_into("../../resources/hextet.neu",mesh));

  boost::shared_ptr< MeshWriter > gmsh_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  BOOST_CHECK_NO_THROW(gmsh_writer->write_from_to(mesh,"hextet.msh"));
  boost::shared_ptr< MeshWriter > neu_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.neu.Writer","meshwriter");
  BOOST_CHECK_NO_THROW(neu_writer->write_from_to(mesh,"hextet_write.neu"));
  BOOST_CHECK_EQUAL(mesh.topology().recursive_nodes_count(), (Uint) 35);
  BOOST_CHECK_EQUAL(mesh.topology().recursive_elements_count(true), (Uint) 44);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( hextet_read_Newneu_writeGmsh )
{
  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");
  boost::shared_ptr< MeshWriter > meshwriter = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");

  // the mesh to store in
  Mesh& mesh = *Core::instance().root().create_component<Mesh>  ( "hextest_write" );

  //CFinfo << "ready to read" << CFendl;
  meshreader->read_mesh_into("hextet_write.neu",mesh);

  //CFinfo << "ready to write" << CFendl;
  meshwriter->write_from_to(mesh,"hextet_write.msh");
  BOOST_CHECK_EQUAL(mesh.topology().recursive_nodes_count(), (Uint) 35);
  BOOST_CHECK_EQUAL(mesh.topology().recursive_elements_count(true), (Uint) 44);

//  boost::shared_ptr< MeshTransformer > meshinfo = build_component_abstract_type<MeshTransformer>("Info","meshinfo");
//  meshinfo->transform(mesh);

}
/*
BOOST_AUTO_TEST_CASE( read_multiple )
{
  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");

  // the file to read from
  boost::filesystem::path fp_in ("quadtriag.neu");

  // the mesh to store in
  boost::shared_ptr< Mesh > mesh ( allocate_component<Mesh>  ( "mesh" ) );

  for (Uint count=1; count<=4; ++count)
  {
    meshreader->read_mesh_into(fp_in,mesh);
    BOOST_CHECK_EQUAL(mesh->domain().recursive_elements_count(true), count*28);
  }

  boost::shared_ptr< MeshTransformer > info  = build_component_abstract_type<MeshTransformer>("Info","info");
  info->transform(mesh);
}*/

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_mesh_signal_2 )
{
  SignalFrame frame;
  SignalOptions options;

  // URI with a wrong protocol
  options.add("location", URI("file:/"));

  frame = options.create_frame("Target", "/", "/");
  BOOST_CHECK_THROW( reader->signal_read(frame), ProtocolError );
}


BOOST_AUTO_TEST_CASE( read_mesh_signal_4 )
{
  SignalFrame frame;
  SignalOptions options;

  // no file (no error and the domain should be still empty afterwards)
  std::vector<URI> files;
  options.add("location", URI("cpath:/MyDom"));
  options.add("files", files);

  frame = options.create_frame("Target", "/", "/");

  std::string str;
  XML::to_string(frame.node, str);
  CFinfo << str << CFendl;

  BOOST_CHECK_THROW( reader->signal_read(frame), BadValue );
  BOOST_CHECK_EQUAL( domain->count_children(), (Uint) 0);
}

BOOST_AUTO_TEST_CASE( read_mesh_signal_5 )
{
  SignalFrame frame;
  SignalOptions options;

  // first file is wrong (exception and the mesh should be empty afterwards)
  std::vector<URI> files;
  files.push_back( "http://www.google.com" );
  files.push_back( "file:../../resources/hextet.neu" );
  options.add("location", URI("cpath:/MyDom"));
  options.add("files", files);

  frame = options.create_frame("Target", "/", "/");

  BOOST_CHECK_THROW( reader->signal_read(frame), ProtocolError );
  BOOST_CHECK_EQUAL( domain->count_children(), (Uint) 0);
}

BOOST_AUTO_TEST_CASE( read_mesh_signal_6 )
{
  SignalFrame frame;
  SignalOptions options;

  // a file in the middle is wrong (exception and the mesh should be empty afterwards)
  std::vector<URI> files;
  files.push_back( "file:../../resources/hextet.neu" );
  files.push_back( "http://www.google.com" );
  files.push_back( "file:../../resources/hextet.neu" );
  options.add("location", URI("cpath:/MyDom"));
  options.add("files", files);

  frame = options.create_frame("Target", "/", "/");

  BOOST_CHECK_THROW( reader->signal_read(frame), ProtocolError );
  BOOST_CHECK_EQUAL( domain->count_children(), (Uint) 0);
}

BOOST_AUTO_TEST_CASE( read_mesh_signal_7 )
{
  SignalFrame frame;
  SignalOptions options;

  // everything is OK
  std::vector<URI> files;
  files.push_back( "file:../../resources/hextet.neu" );
 // more than 1 file no longer supported
//  files.push_back( "file:../../resources/quadtriag.neu" );
  options.add("location", URI("cpath:/MyDom"));
  options.add("files", files);

  frame = options.create_frame("Target", "/", "/");

  BOOST_CHECK_NO_THROW( reader->signal_read(frame) );
  BOOST_CHECK_NE( domain->count_children(), (Uint) 0);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

