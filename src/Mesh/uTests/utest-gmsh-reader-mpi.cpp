// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::Gmsh::CReader"

#include <boost/test/unit_test.hpp>
#include <boost/filesystem/path.hpp>

#include "Common/Log.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshTransformer.hpp"

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
	//PE::instance().init(m_argc,m_argv);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh )
{

  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("Gmsh","meshreader");
	BOOST_CHECK_EQUAL( meshreader->name() , "meshreader" );
	BOOST_CHECK_EQUAL( meshreader->get_format() , "Gmsh" );
	std::vector<std::string> extensions = meshreader->get_extensions();
	BOOST_CHECK_EQUAL( extensions[0] , ".msh" );
	
//	meshreader->configure_property("Repartition",true);
//	meshreader->configure_property("OutputRank",(Uint) 0);
//	meshreader->configure_property("Unified Zones",false);
	
//  // the file to read from
//  //boost::filesystem::path fp_in ("hextet.neu");
//	boost::filesystem::path fp_in ("quadtriag.neu");
	
//  // the mesh to store in
//  CMesh::Ptr mesh ( new CMesh  ( "mesh" ) );
  
	
//	//CFinfo.setFilterRankZero(false);
//  meshreader->read_from_to(fp_in,mesh);
//	//CFinfo.setFilterRankZero(true);
	
//  CFinfo << "elements count = " << get_component_typed<CRegion>(*mesh).recursive_elements_count() << CFendl;
//  CFinfo << "nodes count    = " << get_component_typed<CRegion>(*mesh).recursive_nodes_count() << CFendl;
  
//  Uint nb_ghosts=0;

//  boost::filesystem::path fp_out ("quadtriag.msh");
//  CMeshWriter::Ptr gmsh_writer = create_component_abstract_type<CMeshWriter>("Gmsh","meshwriter");
//  gmsh_writer->write_from_to(mesh,fp_out);
  
//  BOOST_CHECK(true);
	
//	CFinfo << mesh->tree() << CFendl;
	
//	BOOST_FOREACH(CFlexTable& node_2_elems, recursive_filtered_range_typed<CFlexTable >(*mesh,IsComponentName("glb_elem_connectivity")))
//  {
//    CList<bool>& is_ghost = *node_2_elems.look_component_type<CList <bool> >("../is_ghost").get();
//    CList<Uint>& glb_indices = *node_2_elems.look_component_type<CList <Uint> >("../global_indices").get();
    
//    for (Uint i=0; i<node_2_elems.size(); ++i)
//    {
//      if (!is_ghost[i])
//      {
//        CFinfo << "node " << glb_indices[i] << " connected to:" << CFflush;
//        for (Uint j=0; j<node_2_elems.row_size(i); ++j)
//        {
//          CFinfo << " " << node_2_elems[i][j] << CFflush;
//        }
//        CFinfo << CFendl;
//      }
//      else
//      {
//        CFinfo << "node " << glb_indices[i] << " is a ghost node" << CFendl;
//        nb_ghosts++;
//      }
//    }
//    CFinfo << "ghost node count = " << nb_ghosts << CFendl;
//  }
  
//  BOOST_FOREACH(const CList<Uint>& global_element_indices, recursive_filtered_range_typed<CList<Uint> >(*mesh,IsComponentName("global_element_indices")))
//  {
//    Uint local_idx = 0;
//    BOOST_FOREACH(Uint glb_idx, global_element_indices.array())
//      CFinfo << global_element_indices.full_path().string()<<"["<<local_idx++ <<"] = " << glb_idx <<  CFendl;
//  }
	

} 

////////////////////////////////////////////////////////////////////////////////
/*
BOOST_AUTO_TEST_CASE( threeD_test )
{
	
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("Neu","meshreader");
	
	meshreader->configure_property("number_of_processors",(Uint) PE::instance().size());
	meshreader->configure_property("rank",(Uint) PE::instance().rank());
	meshreader->configure_property("Repartition",false);
	meshreader->configure_property("OutputRank",(Uint) 2);
	
  // the file to read from
  boost::filesystem::path fp_in ("hextet.neu");
	
  // the mesh to store in
  CMesh::Ptr mesh ( new CMesh  ( "mesh" ) );
  
	
	CFinfo.setFilterRankZero(false);
  meshreader->read_from_to(fp_in,mesh);
	CFinfo.setFilterRankZero(true);
	
  boost::filesystem::path fp_out ("hextet.msh");
  CMeshWriter::Ptr gmsh_writer = create_component_abstract_type<CMeshWriter>("Gmsh","meshwriter");
  gmsh_writer->write_from_to(mesh,fp_out);
  
  BOOST_CHECK(true);
	
}
*/
////////////////////////////////////////////////////////////////////////////////
/*
BOOST_AUTO_TEST_CASE( read_multiple_2D )
{
	
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("Neu","meshreader");
	
	meshreader->configure_property("Repartition",true);
	meshreader->configure_property("OutputRank",(Uint) 0);
	
  // the file to read from
  boost::filesystem::path fp_in ("quadtriag.neu");
	
  // the mesh to store in
  CMesh::Ptr mesh ( new CMesh  ( "mesh" ) );
  
	
	CFinfo.setFilterRankZero(false);	
	
	

	for (Uint count=1; count<=2; ++count)
	{
		CFinfo << "\n\n\nMesh parallel:" << CFendl;
		meshreader->read_from_to(fp_in,mesh);
	}
	
	
	
	CFinfo.setFilterRankZero(true);
	CFinfo << mesh->tree() << CFendl;
	CFinfo << meshreader->tree() << CFendl;
	CMeshTransformer::Ptr info  = create_component_abstract_type<CMeshTransformer>("Info","info");
	info->transform(mesh);

	
  boost::filesystem::path fp_out ("quadtriag_mult.msh");
  CMeshWriter::Ptr gmsh_writer = create_component_abstract_type<CMeshWriter>("Gmsh","meshwriter");
  gmsh_writer->write_from_to(mesh,fp_out);
  
  BOOST_CHECK_EQUAL(1,1);
	
}
*/
////////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_CASE( finalize_mpi )
{
	//PE::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

