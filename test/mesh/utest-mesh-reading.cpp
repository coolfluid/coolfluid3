
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
#include "common/Core.hpp"
#include "common/Root.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionURI.hpp"

#include "common/FindComponents.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/FileOperations.hpp"
#include "common/XML/SignalFrame.hpp"
#include "common/XML/SignalOptions.hpp"

#include "mesh/CDomain.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/CRegion.hpp"
#include "mesh/CElements.hpp"
#include "mesh/CTable.hpp"
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

    root = Root::create("Root");
    reader = build_component_abstract_type<MeshReader>("CF.Mesh.Neu.CReader","MyReader");
    domain = root->create_component_ptr<CDomain>("MyDom");

    root->add_component( reader );

    ExceptionManager::instance().ExceptionOutputs = false;
    ExceptionManager::instance().ExceptionDumps = false;
  }

  /// common tear-down for each test case
  ~MeshReading_Fixture()
  {
  }

  /// possibly common functions used on the tests below
  Root::Ptr root;
  MeshReader::Ptr reader;
  CDomain::Ptr domain;

  /// common values accessed by all tests goes here
  int    m_argc;
  char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( MeshReading_TestSuite, MeshReading_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Constructors )
{
  MeshReader::Ptr meshreader = build_component_abstract_type<MeshReader>("CF.Mesh.Neu.CReader","meshreader");
  BOOST_CHECK_EQUAL(meshreader->name(),"meshreader");
  BOOST_CHECK_EQUAL(meshreader->get_format(),"Neu");

  MeshWriter::Ptr meshwriter = build_component_abstract_type<MeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  BOOST_CHECK_EQUAL(meshwriter->name(),"meshwriter");
  BOOST_CHECK_EQUAL(meshwriter->get_format(),"Gmsh");

  MeshWriter::Ptr neu_writer = build_component_abstract_type<MeshWriter>("CF.Mesh.Neu.CWriter","meshwriter");
  BOOST_CHECK_EQUAL(neu_writer->name(),"meshwriter");
  BOOST_CHECK_EQUAL(neu_writer->get_format(),"Neu");

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( quadtriag_readNeu_writeGmsh_writeNeu )
{
  MeshReader::Ptr meshreader = build_component_abstract_type<MeshReader>("CF.Mesh.Neu.CReader","meshreader");

  // the mesh to store in
  Mesh& mesh = Core::instance().root().create_component<Mesh>  ( "quadtriag" );

  meshreader->read_mesh_into("quadtriag.neu",mesh);

  BOOST_CHECK(true);
  MeshWriter::Ptr gmsh_writer = build_component_abstract_type<MeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  gmsh_writer->write_from_to(mesh,"quadtriag.msh");
  BOOST_CHECK(true);

  MeshWriter::Ptr neu_writer = build_component_abstract_type<MeshWriter>("CF.Mesh.Neu.CWriter","meshwriter");
  neu_writer->write_from_to(mesh,"quadtriag_write.neu");
  BOOST_CHECK(true);

  BOOST_CHECK_EQUAL(mesh.topology().recursive_nodes_count(), (Uint) 16);
  BOOST_CHECK_EQUAL(mesh.topology().recursive_elements_count(), (Uint) 28);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( quadtriag_read_NewNeu_writeGmsh )
{
  MeshReader::Ptr meshreader = build_component_abstract_type<MeshReader>("CF.Mesh.Neu.CReader","meshreader");
  MeshWriter::Ptr meshwriter = build_component_abstract_type<MeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");

  // the mesh to store in
  Mesh& mesh = Core::instance().root().create_component<Mesh>  ( "quadtriag_write" );

  //CFinfo << "ready to read" << CFendl;
  meshreader->read_mesh_into("quadtriag_write.neu",mesh);

  //CFinfo << "ready to write" << CFendl;
  meshwriter->write_from_to(mesh,"quadtriag_write.msh");
  BOOST_CHECK_EQUAL(mesh.topology().recursive_nodes_count(), (Uint) 16);
  BOOST_CHECK_EQUAL(mesh.topology().recursive_elements_count(), (Uint) 28);

//  MeshTransformer::Ptr meshinfo = build_component_abstract_type<MeshTransformer>("Info","meshinfo");
//  meshinfo->transform(mesh);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( hextet_readNeu_writeGmsh_writeNeu )
{
  MeshReader::Ptr meshreader = build_component_abstract_type<MeshReader>("CF.Mesh.Neu.CReader","meshreader");

  // the mesh to store in
  Mesh& mesh = Core::instance().root().create_component<Mesh>  ( "hextet" );

  meshreader->read_mesh_into("hextet.neu",mesh);

  MeshWriter::Ptr gmsh_writer = build_component_abstract_type<MeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  gmsh_writer->write_from_to(mesh,"hextet.msh");
  MeshWriter::Ptr neu_writer = build_component_abstract_type<MeshWriter>("CF.Mesh.Neu.CWriter","meshwriter");
  neu_writer->write_from_to(mesh,"hextet_write.neu");
  BOOST_CHECK_EQUAL(mesh.topology().recursive_nodes_count(), (Uint) 35);
  BOOST_CHECK_EQUAL(mesh.topology().recursive_elements_count(), (Uint) 44);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( hextet_read_NewNeu_writeGmsh )
{
  MeshReader::Ptr meshreader = build_component_abstract_type<MeshReader>("CF.Mesh.Neu.CReader","meshreader");
  MeshWriter::Ptr meshwriter = build_component_abstract_type<MeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");

  // the mesh to store in
  Mesh& mesh = Core::instance().root().create_component<Mesh>  ( "hextest_write" );

  //CFinfo << "ready to read" << CFendl;
  meshreader->read_mesh_into("hextet_write.neu",mesh);

  //CFinfo << "ready to write" << CFendl;
  meshwriter->write_from_to(mesh,"hextet_write.msh");
  BOOST_CHECK_EQUAL(mesh.topology().recursive_nodes_count(), (Uint) 35);
  BOOST_CHECK_EQUAL(mesh.topology().recursive_elements_count(), (Uint) 44);

//  MeshTransformer::Ptr meshinfo = build_component_abstract_type<MeshTransformer>("Info","meshinfo");
//  meshinfo->transform(mesh);

}
/*
BOOST_AUTO_TEST_CASE( read_multiple )
{
  MeshReader::Ptr meshreader = build_component_abstract_type<MeshReader>("CF.Mesh.Neu.CReader","meshreader");

  // the file to read from
  boost::filesystem::path fp_in ("quadtriag.neu");

  // the mesh to store in
  Mesh::Ptr mesh ( allocate_component<Mesh>  ( "mesh" ) );

  for (Uint count=1; count<=4; ++count)
  {
    meshreader->read_mesh_into(fp_in,mesh);
    BOOST_CHECK_EQUAL(mesh->domain().recursive_elements_count(), count*28);
  }

  MeshTransformer::Ptr info  = build_component_abstract_type<MeshTransformer>("Info","info");
  info->transform(mesh);
}*/

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_mesh_signal_2 )
{
  SignalFrame frame;
  SignalOptions options;

  // URI with a wrong protocol
  options.add_option<OptionURI>("location", URI("file://Root"));

  frame = options.create_frame("Target", "//Root", "//Root");
  BOOST_CHECK_THROW( reader->signal_read(frame), ProtocolError );
}


BOOST_AUTO_TEST_CASE( read_mesh_signal_4 )
{
  SignalFrame frame;
  SignalOptions options;

  // no file (no error and the domain should be still empty afterwards)
  std::vector<URI> files;
  options.add_option<OptionURI>("location", URI("cpath://Root/MyDom"));
  options.add_option<OptionArrayT<URI> >("files", files);

  frame = options.create_frame("Target", "//Root", "//Root");

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
  files.push_back( "file:hextet.neu" );
  options.add_option<OptionURI>("location", URI("cpath://Root/MyDom"));
  options.add_option<OptionArrayT<URI> >("files", files);

  frame = options.create_frame("Target", "//Root", "//Root");

  BOOST_CHECK_THROW( reader->signal_read(frame), ProtocolError );
  BOOST_CHECK_EQUAL( domain->count_children(), (Uint) 0);
}

BOOST_AUTO_TEST_CASE( read_mesh_signal_6 )
{
  SignalFrame frame;
  SignalOptions options;

  // a file in the middle is wrong (exception and the mesh should be empty afterwards)
  std::vector<URI> files;
  files.push_back( "file:hextet.neu" );
  files.push_back( "http://www.google.com" );
  files.push_back( "file:hextet.neu" );
  options.add_option<OptionURI>("location", URI("cpath://Root/MyDom"));
  options.add_option<OptionArrayT<URI> >("files", files);

  frame = options.create_frame("Target", "//Root", "//Root");

  BOOST_CHECK_THROW( reader->signal_read(frame), ProtocolError );
  BOOST_CHECK_EQUAL( domain->count_children(), (Uint) 0);
}

BOOST_AUTO_TEST_CASE( read_mesh_signal_7 )
{
  SignalFrame frame;
  SignalOptions options;

  // everything is OK
  std::vector<URI> files;
  files.push_back( "file:hextet.neu" );
  files.push_back( "file:quadtriag.neu" );
  options.add_option<OptionURI>("location", URI("cpath://Root/MyDom"));
  options.add_option<OptionArrayT<URI> >("files", files);

  frame = options.create_frame("Target", "//Root", "//Root");

  BOOST_CHECK_NO_THROW( reader->signal_read(frame) );
  BOOST_CHECK_NE( domain->count_children(), (Uint) 0);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

