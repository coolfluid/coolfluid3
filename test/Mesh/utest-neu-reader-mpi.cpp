// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::Neu::CReader"

#include <boost/test/unit_test.hpp>
#include <boost/filesystem/path.hpp>

#include "Common/Log.hpp"
#include "Common/CreateComponent.hpp"

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

struct NeuReaderMPITests_Fixture
{
  /// common setup for each test case
  NeuReaderMPITests_Fixture()
  {
		m_argc = boost::unit_test::framework::master_test_suite().argc;
		m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~NeuReaderMPITests_Fixture()
  {
  }
  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here
	int    m_argc;
	char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( NeuReaderMPITests_TestSuite, NeuReaderMPITests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
	//mpi::PE::instance().init(m_argc,m_argv);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh )
{

  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
	
	meshreader->configure_property("Repartition",true);
	meshreader->configure_property("OutputRank",(Uint) 0);
	meshreader->configure_property("Unified Zones",false);
	
  // the file to read from
  //boost::filesystem::path fp_in ("hextet.neu");
	boost::filesystem::path fp_in ("quadtriag.neu");
	
  // the mesh to store in
  CMesh::Ptr mesh ( allocate_component<CMesh>  ( "mesh" ) );
  
	
	//CFinfo.setFilterRankZero(false);
  meshreader->read_from_to(fp_in,mesh);
	//CFinfo.setFilterRankZero(true);
	
  CFinfo << "elements count = " << find_component<CRegion>(*mesh).recursive_elements_count() << CFendl;
  CFinfo << "nodes count    = " << find_component<CRegion>(*mesh).recursive_nodes_count() << CFendl;
  
  Uint nb_ghosts=0;

  boost::filesystem::path fp_out ("quadtriag.msh");
  CMeshWriter::Ptr gmsh_writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  gmsh_writer->write_from_to(mesh,fp_out);
  
  BOOST_CHECK(true);
	
	CFinfo << mesh->tree() << CFendl;
	
  CNodes& nodes = find_component_recursively<CNodes>(*mesh);
  for (Uint n=0; n<nodes.size(); ++n)
  {
    if (!nodes.is_ghost()[n])
    {
      CFinfo << "node " << nodes.glb_idx()[n] << " connected to:" << CFflush;
      boost_foreach (const Uint glb_elem, nodes.glb_elem_connectivity()[n])
        CFinfo << " " << glb_elem << CFflush;
      CFinfo << CFendl;
    }
    else
    {
      CFinfo << "node " << nodes.glb_idx()[n] << " is a ghost node" << CFendl;
      nb_ghosts++;
    }
    CFinfo << "ghost node count = " << nb_ghosts << CFendl;
  }
  
  BOOST_FOREACH(const CList<Uint>& global_element_indices, find_components_recursively_with_name<CList<Uint> >(*mesh,"global_element_indices"))
  {
    Uint local_idx = 0;
    BOOST_FOREACH(Uint glb_idx, global_element_indices.array())
      CFinfo << global_element_indices.full_path().string()<<"["<<local_idx++ <<"] = " << glb_idx <<  CFendl;
  }
	

} 

////////////////////////////////////////////////////////////////////////////////
/*
BOOST_AUTO_TEST_CASE( threeD_test )
{
	
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
	
	meshreader->configure_property("number_of_processors",(Uint) PE::instance().size());
	meshreader->configure_property("rank",(Uint) PE::instance().rank());
	meshreader->configure_property("Repartition",false);
	meshreader->configure_property("OutputRank",(Uint) 2);
	
  // the file to read from
  boost::filesystem::path fp_in ("hextet.neu");
	
  // the mesh to store in
  CMesh::Ptr mesh ( allocate_component<CMesh>  ( "mesh" ) );
  
	
	CFinfo.setFilterRankZero(false);
  meshreader->read_from_to(fp_in,mesh);
	CFinfo.setFilterRankZero(true);
	
  boost::filesystem::path fp_out ("hextet.msh");
  CMeshWriter::Ptr gmsh_writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  gmsh_writer->write_from_to(mesh,fp_out);
  
  BOOST_CHECK(true);
	
}
*/
////////////////////////////////////////////////////////////////////////////////
/*
BOOST_AUTO_TEST_CASE( read_multiple_2D )
{
	
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
	
	meshreader->configure_property("Repartition",true);
	meshreader->configure_property("OutputRank",(Uint) 0);
	
  // the file to read from
  boost::filesystem::path fp_in ("quadtriag.neu");
	
  // the mesh to store in
  CMesh::Ptr mesh ( allocate_component<CMesh>  ( "mesh" ) );
  
	
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
  CMeshWriter::Ptr gmsh_writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  gmsh_writer->write_from_to(mesh,fp_out);
  
  BOOST_CHECK_EQUAL(1,1);
	
}
*/
////////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_CASE( finalize_mpi )
{
	//mpi::PE::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

