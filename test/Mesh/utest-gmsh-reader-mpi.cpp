// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::Gmsh::CReader"

#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"


#include "Common/Core.hpp"
#include "Common/CRoot.hpp"

#include "Math/VariablesDescriptor.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/Field.hpp"
#include "Mesh/CEntities.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/CDynTable.hpp"
#include "Mesh/CList.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/Geometry.hpp"

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

BOOST_AUTO_TEST_CASE( read_2d_mesh_triag_p1 )
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
  CMesh& mesh = Core::instance().root().create_component<CMesh>("mesh_2d_triag_p1");

  // CFinfo.setFilterRankZero(false);
  meshreader->read_mesh_into("rectangle-tg-p1.msh",mesh);
  // CFinfo.setFilterRankZero(true);

  // CFinfo << mesh.tree() << CFendl;

CMeshWriter::Ptr mesh_writer =
    build_component_abstract_type<CMeshWriter> ("CF.Mesh.Gmsh.CWriter", "GmshWriter" );

mesh_writer->write_from_to(mesh,"rectangle-tg-p1-out.msh");

  CFinfo << "elements count = " << find_component<CRegion>(mesh).recursive_elements_count() << CFendl;
  CFinfo << "nodes count    = " << find_component<CRegion>(mesh).recursive_nodes_count() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_triag_p2 )
{

  CMeshReader::Ptr meshreader = build_component_abstract_type<CMeshReader>("CF.Mesh.Gmsh.CReader","meshreader");

  // the mesh to store in
  CMesh& mesh = Core::instance().root().create_component<CMesh>("mesh_2d_triag_p2");

  // CFinfo.setFilterRankZero(false);
  meshreader->read_mesh_into("rectangle-tg-p2.msh",mesh);
  // CFinfo.setFilterRankZero(true);

  // CFinfo << mesh.tree() << CFendl;

CMeshWriter::Ptr mesh_writer =
    build_component_abstract_type<CMeshWriter> ("CF.Mesh.Gmsh.CWriter", "GmshWriter" );
mesh_writer->write_from_to(mesh,"rectangle-tg-p2-out.msh");

  CFinfo << "elements count = " << find_component<CRegion>(mesh).recursive_elements_count() << CFendl;
  CFinfo << "nodes count    = " << find_component<CRegion>(mesh).recursive_nodes_count() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_quad_p1 )
{

  CMeshReader::Ptr meshreader = build_component_abstract_type<CMeshReader>("CF.Mesh.Gmsh.CReader","meshreader");

  // the mesh to store in
  CMesh& mesh = Core::instance().root().create_component<CMesh>("mesh_2d_quad_p1");

  // CFinfo.setFilterRankZero(false);
  meshreader->read_mesh_into("rectangle-qd-p2.msh",mesh);
  // CFinfo.setFilterRankZero(true);

  // CFinfo << mesh.tree() << CFendl;

CMeshWriter::Ptr mesh_writer =
    build_component_abstract_type<CMeshWriter> ("CF.Mesh.Gmsh.CWriter", "GmshWriter" );
mesh_writer->write_from_to(mesh,"rectangle-qd-p1-out.msh");

  CFinfo << "elements count = " << find_component<CRegion>(mesh).recursive_elements_count() << CFendl;
  CFinfo << "nodes count    = " << find_component<CRegion>(mesh).recursive_nodes_count() << CFendl;

}
////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_quad_p2 )
{

  CMeshReader::Ptr meshreader = build_component_abstract_type<CMeshReader>("CF.Mesh.Gmsh.CReader","meshreader");

  // the mesh to store in
  CMesh& mesh = Core::instance().root().create_component<CMesh>("mesh_2d_quad_p2");

  // CFinfo.setFilterRankZero(false);
  meshreader->read_mesh_into("rectangle-qd-p2.msh",mesh);
  // CFinfo.setFilterRankZero(true);

  // CFinfo << mesh.tree() << CFendl;

CMeshWriter::Ptr mesh_writer =
    build_component_abstract_type<CMeshWriter> ("CF.Mesh.Gmsh.CWriter", "GmshWriter" );
mesh_writer->write_from_to(mesh,"rectangle-qd-p2-out.msh");

  CFinfo << "elements count = " << find_component<CRegion>(mesh).recursive_elements_count() << CFendl;
  CFinfo << "nodes count    = " << find_component<CRegion>(mesh).recursive_nodes_count() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_mix_p1 )
{

  CMeshReader::Ptr meshreader = build_component_abstract_type<CMeshReader>("CF.Mesh.Gmsh.CReader","meshreader");

  // the mesh to store in
  CMesh& mesh = Core::instance().root().create_component<CMesh>("mesh_2d_mix_p1");

  // CFinfo.setFilterRankZero(false);
  meshreader->read_mesh_into("rectangle-mix-p1.msh",mesh);
  // CFinfo.setFilterRankZero(true);

  // CFinfo << mesh.tree() << CFendl;

CMeshWriter::Ptr mesh_writer =
    build_component_abstract_type<CMeshWriter> ("CF.Mesh.Gmsh.CWriter", "GmshWriter" );
mesh_writer->write_from_to(mesh,"rectangle-mix-p1-out.msh");

  CFinfo << "elements count = " << find_component<CRegion>(mesh).recursive_elements_count() << CFendl;
  CFinfo << "nodes count    = " << find_component<CRegion>(mesh).recursive_nodes_count() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_mix_p2 )
{

  CMeshReader::Ptr meshreader = build_component_abstract_type<CMeshReader>("CF.Mesh.Gmsh.CReader","meshreader");

  // the mesh to store in
  CMesh& mesh = Core::instance().root().create_component<CMesh>("mesh_2d_mix_p2");

  // CFinfo.setFilterRankZero(false);
  meshreader->read_mesh_into("rectangle-mix-p2.msh",mesh);
  // CFinfo.setFilterRankZero(true);

  // CFinfo << mesh.tree() << CFendl;

CMeshWriter::Ptr mesh_writer =
    build_component_abstract_type<CMeshWriter> ("CF.Mesh.Gmsh.CWriter", "GmshWriter" );
mesh_writer->write_from_to(mesh,"rectangle-mix-p2-out.msh");

  CFinfo << "elements count = " << find_component<CRegion>(mesh).recursive_elements_count() << CFendl;
  CFinfo << "nodes count    = " << find_component<CRegion>(mesh).recursive_nodes_count() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_mix_p1_out )
{
  BOOST_CHECK(true);
  CMeshReader::Ptr meshreader = build_component_abstract_type<CMeshReader>("CF.Mesh.Gmsh.CReader","meshreader");

  // the mesh to store in
  CMesh& mesh = Core::instance().root().create_component<CMesh>("mesh_2d_mix_p1_out");

  // CFinfo.setFilterRankZero(false);
  meshreader->read_mesh_into("rectangle-mix-p1-out.msh",mesh);
  // CFinfo.setFilterRankZero(true);
  BOOST_CHECK(true);

  // CFinfo << mesh.tree() << CFendl;

  Field& nodal = mesh.geometry().create_field("nodal","nodal[vector]",DIM_2D);
  nodal.descriptor().configure_option("dimension",mesh.dimension());
  for (Uint n=0; n<nodal.size(); ++n)
  {
    for(Uint j=0; j<nodal.row_size(); ++j)
      nodal[n][j] = n;
  }

  boost_foreach(CEntities& elements, mesh.topology().elements_range())
    elements.create_space("elems_P0","CF.Mesh.SF.SF"+elements.element_type().shape_name()+"LagrangeP0");
  mesh.create_field_group("elems_P0",FieldGroup::Basis::ELEMENT_BASED);

  Field& cell_centred = mesh.geometry().create_field("cell_centred","cell_centred[vector]",mesh.dimension());
  for (Uint e=0; e<cell_centred.size(); ++e)
  {
    for(Uint j=0; j<cell_centred.row_size(); ++j)
      cell_centred[e][j] = e;
  }

  std::vector<Field::Ptr> fields;
  fields.push_back(nodal.as_ptr<Field>());
  fields.push_back(cell_centred.as_ptr<Field>());


  CMeshWriter::Ptr mesh_writer =
    build_component_abstract_type<CMeshWriter> ("CF.Mesh.Gmsh.CWriter", "GmshWriter" );
  mesh_writer->set_fields(fields);
  mesh_writer->write_from_to(mesh,"rectangle-mix-p1-out-out.msh");

  BOOST_CHECK(true);

  CFinfo << "elements count = " << find_component<CRegion>(mesh).recursive_elements_count() << CFendl;
  CFinfo << "nodes count    = " << find_component<CRegion>(mesh).recursive_nodes_count() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
  Core::instance().terminate();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

