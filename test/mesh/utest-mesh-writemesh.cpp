// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::mesh::tecplot::Writer"

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/Root.hpp"

#include "math/VariablesDescriptor.hpp"

#include "mesh/Domain.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/Field.hpp"
#include "common/DynTable.hpp"
#include "common/List.hpp"
#include "common/Table.hpp"
#include "mesh/SpaceFields.hpp"
#include "mesh/WriteMesh.hpp"

using namespace std;
using namespace boost;
using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

struct TecWriterTests_Fixture
{
  /// common setup for each test case
  TecWriterTests_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~TecWriterTests_Fixture()
  {
  }
  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here
  int    m_argc;
  char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( TecWriterTests_TestSuite, TecWriterTests_Fixture )

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh )
{

  MeshReader::Ptr meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");

  meshreader->configure_option("read_groups",true);

  Domain& domain = Core::instance().root().create_component<Domain>("domain");
  // the mesh to store in
  Mesh& mesh = domain.create_component<Mesh>( "mesh" );

  meshreader->read_mesh_into("../../resources/quadtriag.neu",mesh);


  Field& nodal = mesh.geometry_fields().create_field("nodal","nodal[vector]");
  nodal.descriptor().configure_option(common::Tags::dimension(),mesh.dimension());
  for (Uint n=0; n<nodal.size(); ++n)
  {
    for(Uint j=0; j<nodal.row_size(); ++j)
      nodal[n][j] = n;
  }

  SpaceFields& elems_P0 = mesh.create_space_and_field_group("elems_P0",SpaceFields::Basis::ELEMENT_BASED,"cf3.mesh.LagrangeP0");

  Field& cell_centred = elems_P0.create_field("cell_centred","cell_centred[vector]");
  for (Uint e=0; e<cell_centred.size(); ++e)
  {
    for(Uint j=0; j<cell_centred.row_size(); ++j)
      cell_centred[e][j] = e;
  }

  std::vector<URI> fields;
  fields.push_back(nodal.uri());
  fields.push_back(cell_centred.uri());
  fields.push_back(mesh.geometry_fields().coordinates().uri());

  WriteMesh& write_mesh = Core::instance().root().create_component<WriteMesh>("write_mesh");
  write_mesh.write_mesh(mesh,"quadtriag.plt",fields);
  write_mesh.write_mesh(mesh,"quadtriag.msh",fields);

  BOOST_CHECK(true);

  domain.write_mesh("quadtriag.plt");
  domain.write_mesh("quadtriag.msh");
}

////////////////////////////////////////////////////////////////////////////////
/*
BOOST_AUTO_TEST_CASE( threeD_test )
{

  MeshReader::Ptr meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");

  meshreader->configure_option("number_of_processors",(Uint) Comm::instance().size());
  meshreader->configure_option("rank",(Uint) Comm::instance().rank());
  meshreader->configure_option("Repartition",false);
  meshreader->configure_option("OutputRank",(Uint) 2);

  // the file to read from
  boost::filesystem::path fp_in ("../../resources/hextet.neu");

  // the mesh to store in
  Mesh::Ptr mesh ( allocate_component<Mesh>  ( "mesh" ) );


  CFinfo.setFilterRankZero(false);
  meshreader->do_read_mesh_into(fp_in,mesh);
  CFinfo.setFilterRankZero(true);

  boost::filesystem::path fp_out ("hextet.msh");
  MeshWriter::Ptr gmsh_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  gmsh_writer->write_from_to(mesh,fp_out);

  BOOST_CHECK(true);

}
*/
////////////////////////////////////////////////////////////////////////////////
/*
BOOST_AUTO_TEST_CASE( read_multiple_2D )
{

  MeshReader::Ptr meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");

  meshreader->configure_option("Repartition",true);
  meshreader->configure_option("OutputRank",(Uint) 0);

  // the file to read from
  boost::filesystem::path fp_in ("quadtriag.neu");

  // the mesh to store in
  Mesh::Ptr mesh ( allocate_component<Mesh>  ( "mesh" ) );


  CFinfo.setFilterRankZero(false);



  for (Uint count=1; count<=2; ++count)
  {
    CFinfo << "\n\n\nMesh parallel:" << CFendl;
    meshreader->do_read_mesh_into(fp_in,mesh);
  }



  CFinfo.setFilterRankZero(true);
  CFinfo << mesh->tree() << CFendl;
  CFinfo << meshreader->tree() << CFendl;
  MeshTransformer::Ptr info  = build_component_abstract_type<MeshTransformer>("Info","info");
  info->transform(mesh);


  boost::filesystem::path fp_out ("quadtriag_mult.msh");
  MeshWriter::Ptr gmsh_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  gmsh_writer->write_from_to(mesh,fp_out);

  BOOST_CHECK_EQUAL(1,1);

}
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

