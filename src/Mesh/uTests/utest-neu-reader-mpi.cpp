// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::Neu::CReader"

#include <boost/test/unit_test.hpp>
#include <boost/filesystem/path.hpp>

#include "Common/ConfigObject.hpp"
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
	PEInterface::instance().init(m_argc,m_argv);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( twoD_test )
{

  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("Neu","meshreader");
	
	meshreader->configure_property("Repartition",true);
	meshreader->configure_property("OutputRank",(Uint) 0);
	
  // the file to read from
	boost::filesystem::path fp_in ("quadtriag.neu");
	
  // the mesh to store in
  CMesh::Ptr mesh ( new CMesh  ( "mesh" ) );
  
	
	//CFinfo.setFilterRankZero(false);
  meshreader->read_from_to(fp_in,mesh);
	//CFinfo.setFilterRankZero(true);
	
  boost::filesystem::path fp_out ("quadtriag.msh");
  CMeshWriter::Ptr gmsh_writer = create_component_abstract_type<CMeshWriter>("Gmsh","meshwriter");
  gmsh_writer->write_from_to(mesh,fp_out);
  
  BOOST_CHECK(true);
	
	CFinfo << mesh->tree() << CFendl;

} 

////////////////////////////////////////////////////////////////////////////////
/*
BOOST_AUTO_TEST_CASE( threeD_test )
{
	
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("Neu","meshreader");
	
	meshreader->configure_property("number_of_processors",(Uint) PEInterface::instance().size());
	meshreader->configure_property("rank",(Uint) PEInterface::instance().rank());
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

////////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_CASE( finalize_mpi )
{
	PEInterface::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

