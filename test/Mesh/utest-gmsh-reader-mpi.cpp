// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::Gmsh::CReader"

#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
 

#include "Common/Core.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshTransformer.hpp"

#include "Mesh/CDynTable.hpp"
#include "Mesh/CList.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CNodes.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

struct GmshReaderMPITests_Fixture
{
  /// common setup for each test case
  GmshReaderMPITests_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~GmshReaderMPITests_Fixture()
  {
  }
  /// possibly common functions used on the tests below


	/// common values accessed by all tests goes here
	int    m_argc;
	char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( GmshReaderMPITests_TestSuite, GmshReaderMPITests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
	Core::instance().initiate(m_argc,m_argv);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_triag )
{

	CMeshReader::Ptr meshreader = build_component_abstract_type<CMeshReader>("CF.Mesh.Gmsh.CReader","meshreader");
	BOOST_CHECK_EQUAL( meshreader->name() , "meshreader" );
	BOOST_CHECK_EQUAL( meshreader->get_format() , "Gmsh" );
	std::vector<std::string> extensions = meshreader->get_extensions();
	BOOST_CHECK_EQUAL( extensions[0] , ".msh" );

//	meshreader->configure_option("Repartition",true);
//	meshreader->configure_option("OutputRank",(Uint) 0);
//	meshreader->configure_option("Unified Zones",false);


  // the mesh to store in
  CMesh::Ptr mesh ( allocate_component<CMesh>  ( "mesh" ) );

  // CFinfo.setFilterRankZero(false);
  meshreader->read_mesh_into("rectangle-tg-p1.msh",*mesh);
  // CFinfo.setFilterRankZero(true);

  // CFinfo << mesh->tree() << CFendl;

CMeshWriter::Ptr mesh_writer =
    build_component_abstract_type<CMeshWriter> ("CF.Mesh.Gmsh.CWriter", "GmshWriter" );

mesh_writer->write_from_to(*mesh,"rectangle-tg-p1-out.msh");

  CFinfo << "elements count = " << find_component<CRegion>(*mesh).recursive_elements_count() << CFendl;
  CFinfo << "nodes count    = " << find_component<CRegion>(*mesh).recursive_nodes_count() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_triag_p2 )
{

  CMeshReader::Ptr meshreader = build_component_abstract_type<CMeshReader>("CF.Mesh.Gmsh.CReader","meshreader");

  // the mesh to store in
  CMesh::Ptr mesh ( allocate_component<CMesh>  ( "mesh" ) );

  // CFinfo.setFilterRankZero(false);
  meshreader->read_mesh_into("rectangle-tg-p2.msh",*mesh);
  // CFinfo.setFilterRankZero(true);

  // CFinfo << mesh->tree() << CFendl;

CMeshWriter::Ptr mesh_writer =
    build_component_abstract_type<CMeshWriter> ("CF.Mesh.Gmsh.CWriter", "GmshWriter" );
mesh_writer->write_from_to(*mesh,"rectangle-tg-p2-out.msh");

  CFinfo << "elements count = " << find_component<CRegion>(*mesh).recursive_elements_count() << CFendl;
  CFinfo << "nodes count    = " << find_component<CRegion>(*mesh).recursive_nodes_count() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_quad_p1 )
{

  CMeshReader::Ptr meshreader = build_component_abstract_type<CMeshReader>("CF.Mesh.Gmsh.CReader","meshreader");

  // the mesh to store in
  CMesh::Ptr mesh ( allocate_component<CMesh>  ( "mesh" ) );

  // CFinfo.setFilterRankZero(false);
  meshreader->read_mesh_into("rectangle-qd-p2.msh",*mesh);
  // CFinfo.setFilterRankZero(true);

  // CFinfo << mesh->tree() << CFendl;

CMeshWriter::Ptr mesh_writer =
    build_component_abstract_type<CMeshWriter> ("CF.Mesh.Gmsh.CWriter", "GmshWriter" );
mesh_writer->write_from_to(*mesh,"rectangle-qd-p1-out.msh");

  CFinfo << "elements count = " << find_component<CRegion>(*mesh).recursive_elements_count() << CFendl;
  CFinfo << "nodes count    = " << find_component<CRegion>(*mesh).recursive_nodes_count() << CFendl;

}
////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_quad_p2 )
{

  CMeshReader::Ptr meshreader = build_component_abstract_type<CMeshReader>("CF.Mesh.Gmsh.CReader","meshreader");

  // the mesh to store in
  CMesh::Ptr mesh ( allocate_component<CMesh>  ( "mesh" ) );

  // CFinfo.setFilterRankZero(false);
  meshreader->read_mesh_into("rectangle-qd-p2.msh",*mesh);
  // CFinfo.setFilterRankZero(true);

  // CFinfo << mesh->tree() << CFendl;

CMeshWriter::Ptr mesh_writer =
    build_component_abstract_type<CMeshWriter> ("CF.Mesh.Gmsh.CWriter", "GmshWriter" );
mesh_writer->write_from_to(*mesh,"rectangle-qd-p2-out.msh");

  CFinfo << "elements count = " << find_component<CRegion>(*mesh).recursive_elements_count() << CFendl;
  CFinfo << "nodes count    = " << find_component<CRegion>(*mesh).recursive_nodes_count() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_mix_p1 )
{

  CMeshReader::Ptr meshreader = build_component_abstract_type<CMeshReader>("CF.Mesh.Gmsh.CReader","meshreader");

  // the mesh to store in
  CMesh::Ptr mesh ( allocate_component<CMesh>  ( "mesh" ) );

  // CFinfo.setFilterRankZero(false);
  meshreader->read_mesh_into("rectangle-mix-p1.msh",*mesh);
  // CFinfo.setFilterRankZero(true);

  // CFinfo << mesh->tree() << CFendl;

CMeshWriter::Ptr mesh_writer =
    build_component_abstract_type<CMeshWriter> ("CF.Mesh.Gmsh.CWriter", "GmshWriter" );
mesh_writer->write_from_to(*mesh,"rectangle-mix-p1-out.msh");

  CFinfo << "elements count = " << find_component<CRegion>(*mesh).recursive_elements_count() << CFendl;
  CFinfo << "nodes count    = " << find_component<CRegion>(*mesh).recursive_nodes_count() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_mix_p2 )
{

  CMeshReader::Ptr meshreader = build_component_abstract_type<CMeshReader>("CF.Mesh.Gmsh.CReader","meshreader");

  // the mesh to store in
  CMesh::Ptr mesh ( allocate_component<CMesh>  ( "mesh" ) );

  // CFinfo.setFilterRankZero(false);
  meshreader->read_mesh_into("rectangle-mix-p2.msh",*mesh);
  // CFinfo.setFilterRankZero(true);

  // CFinfo << mesh->tree() << CFendl;

CMeshWriter::Ptr mesh_writer =
    build_component_abstract_type<CMeshWriter> ("CF.Mesh.Gmsh.CWriter", "GmshWriter" );
mesh_writer->write_from_to(*mesh,"rectangle-mix-p2-out.msh");

  CFinfo << "elements count = " << find_component<CRegion>(*mesh).recursive_elements_count() << CFendl;
  CFinfo << "nodes count    = " << find_component<CRegion>(*mesh).recursive_nodes_count() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_mix_p1_out )
{
  BOOST_CHECK(true);
  CMeshReader::Ptr meshreader = build_component_abstract_type<CMeshReader>("CF.Mesh.Gmsh.CReader","meshreader");

  // the mesh to store in
  CMesh::Ptr mesh ( allocate_component<CMesh>  ( "mesh" ) );

  // CFinfo.setFilterRankZero(false);
  meshreader->read_mesh_into("rectangle-mix-p1-out.msh",*mesh);
  // CFinfo.setFilterRankZero(true);
  BOOST_CHECK(true);

  // CFinfo << mesh->tree() << CFendl;

CMeshWriter::Ptr mesh_writer =
    build_component_abstract_type<CMeshWriter> ("CF.Mesh.Gmsh.CWriter", "GmshWriter" );
mesh_writer->write_from_to(*mesh,"rectangle-mix-p1-out-out.msh");
BOOST_CHECK(true);

  CFinfo << "elements count = " << find_component<CRegion>(*mesh).recursive_elements_count() << CFendl;
  CFinfo << "nodes count    = " << find_component<CRegion>(*mesh).recursive_nodes_count() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
  Core::instance().terminate();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

