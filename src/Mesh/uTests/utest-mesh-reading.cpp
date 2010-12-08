
// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Tests mesh reading"

#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/regex.hpp>

#include "Common/OptionT.hpp"
#include "Common/Log.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/CreateComponent.hpp"

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
    domain = root->create_component_type<CDomain>("MyDom");

    root->add_component( reader );
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
  CMesh::Ptr mesh ( allocate_component_type<CMesh>  ( "mesh" ) );

  meshreader->read_from_to(fp_in,mesh);

  boost::filesystem::path fp_out ("quadtriag.msh");
  CMeshWriter::Ptr gmsh_writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  gmsh_writer->write_from_to(mesh,fp_out);
  boost::filesystem::path fp_out_neu ("quadtriag_write.neu");
  CMeshWriter::Ptr neu_writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Neu.CWriter","meshwriter");
  neu_writer->write_from_to(mesh,fp_out_neu);

  BOOST_CHECK_EQUAL(mesh->domain().recursive_nodes_count(), (Uint) 16);
  BOOST_CHECK_EQUAL(mesh->domain().recursive_elements_count(), (Uint) 28);
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
  CMesh::Ptr mesh ( allocate_component_type<CMesh>  ( "mesh" ) );

  //CFinfo << "ready to read" << CFendl;
  meshreader->read_from_to(fp_in,mesh);

  //CFinfo << "ready to write" << CFendl;
  meshwriter->write_from_to(mesh,fp_out);
  BOOST_CHECK_EQUAL(mesh->domain().recursive_nodes_count(), (Uint) 16);
  BOOST_CHECK_EQUAL(mesh->domain().recursive_elements_count(), (Uint) 28);

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
  CMesh::Ptr mesh ( allocate_component_type<CMesh>  ( "mesh" ) );

  meshreader->read_from_to(fp_in,mesh);

  boost::filesystem::path fp_out ("hextet.msh");
  CMeshWriter::Ptr gmsh_writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  gmsh_writer->write_from_to(mesh,fp_out);
  boost::filesystem::path fp_out_neu ("hextet_write.neu");
  CMeshWriter::Ptr neu_writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Neu.CWriter","meshwriter");
  neu_writer->write_from_to(mesh,fp_out_neu);
  BOOST_CHECK_EQUAL(mesh->domain().recursive_nodes_count(), (Uint) 35);
  BOOST_CHECK_EQUAL(mesh->domain().recursive_elements_count(), (Uint) 44);
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
  CMesh::Ptr mesh ( allocate_component_type<CMesh>  ( "mesh" ) );

  //CFinfo << "ready to read" << CFendl;
  meshreader->read_from_to(fp_in,mesh);

  //CFinfo << "ready to write" << CFendl;
  meshwriter->write_from_to(mesh,fp_out);
  BOOST_CHECK_EQUAL(mesh->domain().recursive_nodes_count(), (Uint) 35);
  BOOST_CHECK_EQUAL(mesh->domain().recursive_elements_count(), (Uint) 44);

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
  CMesh::Ptr mesh ( allocate_component_type<CMesh>  ( "mesh" ) );

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
  boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
  XmlNode& node  = *XmlOps::goto_doc_node(*doc.get());
  XmlParams p(node);

  // without CPath for the CDomain
  p.add_option<URI>("Domain", URI("//Root"));
  BOOST_CHECK_THROW( reader->read(node), ProtocolError );
}

BOOST_AUTO_TEST_CASE( read_mesh_signal_2 )
{
  boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
  XmlNode& node  = *XmlOps::goto_doc_node(*doc.get());
  XmlParams p(node);

  // URI with a wrong protocol
  p.add_option<URI>("Domain", URI("file://Root"));
  BOOST_CHECK_THROW( reader->read(node), ProtocolError );
}

BOOST_AUTO_TEST_CASE( read_mesh_signal_3 )
{
  boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
  XmlNode& node  = *XmlOps::goto_doc_node(*doc.get());
  XmlParams p(node);

  // CPath that does not point to a CDomain
  p.add_option<URI>("Domain", URI("cpath://Root"));
  BOOST_CHECK_THROW( reader->read(node), CastingFailed );
}


BOOST_AUTO_TEST_CASE( read_mesh_signal_4 )
{
  boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
  XmlNode& node  = *XmlOps::goto_doc_node(*doc.get());
  XmlParams p(node);

  // no file (no error and the domain should be still empty afterwards)
  std::vector<URI> files;
  p.add_option<URI>("Domain", URI("cpath://Root/MyDom"));
  p.add_array("Files", files);
  BOOST_CHECK_THROW( reader->read(node), BadValue );
  BOOST_CHECK_EQUAL( domain->get_child_count(), (Uint) 0);
}

BOOST_AUTO_TEST_CASE( read_mesh_signal_5 )
{
  boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
  XmlNode& node  = *XmlOps::goto_doc_node(*doc.get());
  XmlParams p(node);

  // first file is wrong (exception and the mesh should be empty afterwards)
  std::vector<URI> files;
  files.push_back( "http://www.google.com" );
  files.push_back( "file:hextet.neu" );
  p.add_option<URI>("Domain", URI("cpath://Root/MyDom"));
  p.add_array("Files", files);
  BOOST_CHECK_THROW( reader->read(node), ProtocolError );
  BOOST_CHECK_EQUAL( domain->get_child_count(), (Uint) 0);
}

BOOST_AUTO_TEST_CASE( read_mesh_signal_6 )
{
  boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
  XmlNode& node  = *XmlOps::goto_doc_node(*doc.get());
  XmlParams p(node);

  // a file in the middle is wrong (exception and the mesh should be empty afterwards)
  std::vector<URI> files;
  files.push_back( "file:hextet.neu" );
  files.push_back( "http://www.google.com" );
  files.push_back( "file:hextet.neu" );
  p.add_option<URI>("Domain", URI("cpath://Root/MyDom"));
  p.add_array("Files", files);
  BOOST_CHECK_THROW( reader->read(node), ProtocolError );
  BOOST_CHECK_EQUAL( domain->get_child_count(), (Uint) 0);
}

BOOST_AUTO_TEST_CASE( read_mesh_signal_7 )
{
  boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
  XmlNode& node  = *XmlOps::goto_doc_node(*doc.get());
  XmlParams p(node);

  // a file in the middle is wrong (exception and the mesh should be empty afterwards)
  std::vector<URI> files;
  files.push_back( "file:hextet.neu" );
  files.push_back( "file:quadtriag.neu" );
  p.add_option<URI>("Domain", URI("cpath://Root/MyDom"));
  p.add_array("Files", files);
  BOOST_CHECK_NO_THROW( reader->read(node) );
  BOOST_CHECK_NE( domain->get_child_count(), (Uint) 0);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

