
// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh reading"

#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>

#include "Common/OptionT.hpp"
#include "Common/Log.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/XML/Protocol.hpp"

#include "Mesh/CDomain.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshTransformer.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;
using namespace CF::Common::XML;

////////////////////////////////////////////////////////////////////////////////

struct MeshReading_Fixture
{
  /// common setup for each test case
  MeshReading_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;

    root = CRoot::create("Root");
    reader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","MyReader");
    domain = root->create_component<CDomain>("MyDom");

    root->add_component( reader );

    ExceptionManager::instance().ExceptionOutputs = false;
    ExceptionManager::instance().ExceptionDumps = false;
  }

  /// common tear-down for each test case
  ~MeshReading_Fixture()
  {
  }

  /// possibly common functions used on the tests below
  CRoot::Ptr root;
  CMeshReader::Ptr reader;
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
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
  BOOST_CHECK_EQUAL(meshreader->name(),"meshreader");
  BOOST_CHECK_EQUAL(meshreader->get_format(),"Neu");

  CMeshWriter::Ptr meshwriter = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  BOOST_CHECK_EQUAL(meshwriter->name(),"meshwriter");
  BOOST_CHECK_EQUAL(meshwriter->get_format(),"Gmsh");

  CMeshWriter::Ptr neu_writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Neu.CWriter","meshwriter");
  BOOST_CHECK_EQUAL(neu_writer->name(),"meshwriter");
  BOOST_CHECK_EQUAL(neu_writer->get_format(),"Neu");

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( quadtriag_readNeu_writeGmsh_writeNeu )
{
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");

  // the file to read from
  boost::filesystem::path fp_in ("quadtriag.neu");

  // the mesh to store in
  CMesh::Ptr mesh ( allocate_component<CMesh>  ( "mesh" ) );

  meshreader->read_from_to(fp_in,mesh);

  boost::filesystem::path fp_out ("quadtriag.msh");
  CMeshWriter::Ptr gmsh_writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  gmsh_writer->write_from_to(mesh,fp_out);
  boost::filesystem::path fp_out_neu ("quadtriag_write.neu");
  CMeshWriter::Ptr neu_writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Neu.CWriter","meshwriter");
  neu_writer->write_from_to(mesh,fp_out_neu);

  BOOST_CHECK_EQUAL(mesh->topology().recursive_nodes_count(), (Uint) 16);
  BOOST_CHECK_EQUAL(mesh->topology().recursive_elements_count(), (Uint) 28);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( quadtriag_read_NewNeu_writeGmsh )
{
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
  CMeshWriter::Ptr meshwriter = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");

  // the file to read from and to
  boost::filesystem::path fp_in ("quadtriag_write.neu");
  boost::filesystem::path fp_out("quadtriag_write.msh");

  // the mesh to store in
  CMesh::Ptr mesh ( allocate_component<CMesh>  ( "mesh" ) );

  //CFinfo << "ready to read" << CFendl;
  meshreader->read_from_to(fp_in,mesh);

  //CFinfo << "ready to write" << CFendl;
  meshwriter->write_from_to(mesh,fp_out);
  BOOST_CHECK_EQUAL(mesh->topology().recursive_nodes_count(), (Uint) 16);
  BOOST_CHECK_EQUAL(mesh->topology().recursive_elements_count(), (Uint) 28);

//  CMeshTransformer::Ptr meshinfo = create_component_abstract_type<CMeshTransformer>("Info","meshinfo");
//  meshinfo->transform(mesh);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( hextet_readNeu_writeGmsh_writeNeu )
{
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");

  // the file to read from
  boost::filesystem::path fp_in ("hextet.neu");

  // the mesh to store in
  CMesh::Ptr mesh ( allocate_component<CMesh>  ( "mesh" ) );

  meshreader->read_from_to(fp_in,mesh);

  boost::filesystem::path fp_out ("hextet.msh");
  CMeshWriter::Ptr gmsh_writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  gmsh_writer->write_from_to(mesh,fp_out);
  boost::filesystem::path fp_out_neu ("hextet_write.neu");
  CMeshWriter::Ptr neu_writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Neu.CWriter","meshwriter");
  neu_writer->write_from_to(mesh,fp_out_neu);
  BOOST_CHECK_EQUAL(mesh->topology().recursive_nodes_count(), (Uint) 35);
  BOOST_CHECK_EQUAL(mesh->topology().recursive_elements_count(), (Uint) 44);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( hextet_read_NewNeu_writeGmsh )
{
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
  CMeshWriter::Ptr meshwriter = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");

  // the file to read from and to
  boost::filesystem::path fp_in ("hextet_write.neu");
  boost::filesystem::path fp_out("hextet_write.msh");

  // the mesh to store in
  CMesh::Ptr mesh ( allocate_component<CMesh>  ( "mesh" ) );

  //CFinfo << "ready to read" << CFendl;
  meshreader->read_from_to(fp_in,mesh);

  //CFinfo << "ready to write" << CFendl;
  meshwriter->write_from_to(mesh,fp_out);
  BOOST_CHECK_EQUAL(mesh->topology().recursive_nodes_count(), (Uint) 35);
  BOOST_CHECK_EQUAL(mesh->topology().recursive_elements_count(), (Uint) 44);

//  CMeshTransformer::Ptr meshinfo = create_component_abstract_type<CMeshTransformer>("Info","meshinfo");
//  meshinfo->transform(mesh);

}
/*
BOOST_AUTO_TEST_CASE( read_multiple )
{
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");

  // the file to read from
  boost::filesystem::path fp_in ("quadtriag.neu");

  // the mesh to store in
  CMesh::Ptr mesh ( allocate_component<CMesh>  ( "mesh" ) );

  for (Uint count=1; count<=4; ++count)
  {
    meshreader->read_from_to(fp_in,mesh);
    BOOST_CHECK_EQUAL(mesh->domain().recursive_elements_count(), count*28);
  }

  CMeshTransformer::Ptr info  = create_component_abstract_type<CMeshTransformer>("Info","info");
  info->transform(mesh);
}*/

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_mesh_signal_1 )
{
  SignalFrame frame("Target", "//Root", "//Root");
  SignalFrame& options = frame.map( Protocol::Tags::key_options() );

  // with a wrong CPath for the CDomain
  options.set_option<URI>("Domain", URI("//Root"));
  BOOST_CHECK_THROW( reader->signal_read(frame), CastingFailed );
}

BOOST_AUTO_TEST_CASE( read_mesh_signal_2 )
{
  SignalFrame frame("Target", "//Root", "//Root");
  SignalFrame& options = frame.map( Protocol::Tags::key_options() );

  // URI with a wrong protocol
  options.set_option<URI>("Domain", URI("file://Root"));
  BOOST_CHECK_THROW( reader->signal_read(frame), ProtocolError );
}

BOOST_AUTO_TEST_CASE( read_mesh_signal_3 )
{
  SignalFrame frame("Target", "//Root", "//Root");
  SignalFrame& options = frame.map( Protocol::Tags::key_options() );

  // CPath that does not point to a CDomain
  options.set_option<URI>("Domain", URI("cpath://Root"));
  BOOST_CHECK_THROW( reader->signal_read(frame), CastingFailed );
}


BOOST_AUTO_TEST_CASE( read_mesh_signal_4 )
{
  SignalFrame frame("Target", "//Root", "//Root");
  SignalFrame& options = frame.map( Protocol::Tags::key_options() );

  // no file (no error and the domain should be still empty afterwards)
  std::vector<URI> files;
  options.set_option<URI>("Domain", URI("cpath://Root/MyDom"));
  options.set_array("Files", files, " ; ");

  std::string str;
  frame.node.to_string(str);
  CFinfo << str << CFendl;

  BOOST_CHECK_THROW( reader->signal_read(frame), BadValue );
  BOOST_CHECK_EQUAL( domain->count_children(), (Uint) 0);
}

BOOST_AUTO_TEST_CASE( read_mesh_signal_5 )
{
  SignalFrame frame("Target", "//Root", "//Root");
  SignalFrame& options = frame.map( Protocol::Tags::key_options() );

  // first file is wrong (exception and the mesh should be empty afterwards)
  std::vector<URI> files;
  files.push_back( "http://www.google.com" );
  files.push_back( "file:hextet.neu" );
  options.set_option<URI>("Domain", URI("cpath://Root/MyDom"));
  options.set_array("Files", files, " ; ");
  BOOST_CHECK_THROW( reader->signal_read(frame), ProtocolError );
  BOOST_CHECK_EQUAL( domain->count_children(), (Uint) 0);
}

BOOST_AUTO_TEST_CASE( read_mesh_signal_6 )
{
  SignalFrame frame("Target", "//Root", "//Root");
  SignalFrame& options = frame.map( Protocol::Tags::key_options() );

  // a file in the middle is wrong (exception and the mesh should be empty afterwards)
  std::vector<URI> files;
  files.push_back( "file:hextet.neu" );
  files.push_back( "http://www.google.com" );
  files.push_back( "file:hextet.neu" );
  options.set_option<URI>("Domain", URI("cpath://Root/MyDom"));
  options.set_array("Files", files, " ; ");
  BOOST_CHECK_THROW( reader->signal_read(frame), ProtocolError );
  BOOST_CHECK_EQUAL( domain->count_children(), (Uint) 0);
}

BOOST_AUTO_TEST_CASE( read_mesh_signal_7 )
{
  SignalFrame frame("Target", "//Root", "//Root");
  SignalFrame& options = frame.map( Protocol::Tags::key_options() );

  // everything is OK
  std::vector<URI> files;
  files.push_back( "file:hextet.neu" );
  files.push_back( "file:quadtriag.neu" );
  options.set_option<URI>("Domain", URI("cpath://Root/MyDom"));
  options.set_array("Files", files, " ; ");
  BOOST_CHECK_NO_THROW( reader->signal_read(frame) );
  BOOST_CHECK_NE( domain->count_children(), (Uint) 0);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

