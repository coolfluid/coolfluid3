// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::mesh::tecplot::Writer"

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Core.hpp"

#include "math/VariablesDescriptor.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/Field.hpp"
#include "common/DynTable.hpp"
#include "common/List.hpp"
#include "common/Table.hpp"
#include "mesh/Dictionary.hpp"

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

  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");

  meshreader->options().set("read_groups",true);

  // the mesh to store in
  Mesh& mesh = *Core::instance().root().create_component<Mesh>( "mesh" );

  meshreader->read_mesh_into("../../resources/quadtriag.neu",mesh);


  Uint nb_ghosts=0;


  Field& nodal = mesh.geometry_fields().create_field("nodal","nodal[vector]");
  for (Uint n=0; n<nodal.size(); ++n)
  {
    for(Uint j=0; j<nodal.row_size(); ++j)
      nodal[n][j] = n;
  }


  Dictionary& elems = mesh.create_discontinuous_space("elems_P0","cf3.mesh.LagrangeP0");

  Field& cell_centred = elems.create_field("cell_centred","cell_centred[vector]");
  for (Uint e=0; e<cell_centred.size(); ++e)
  {
    for(Uint j=0; j<cell_centred.row_size(); ++j)
      cell_centred[e][j] = e;
  }


  Dictionary& P2 = mesh.create_continuous_space("nodes_P2","cf3.mesh.LagrangeP2");

  Field& nodesP2 = P2.create_field("nodesP2","nodesP2[vector]");
  for (Uint e=0; e<nodesP2.size(); ++e)
  {
    for(Uint j=0; j<nodesP2.row_size(); ++j)
      nodesP2[e][j] = nodesP2.coordinates()[e][j];
  }


  std::vector<URI> fields;
  fields.push_back(nodal.uri());
  fields.push_back(cell_centred.uri());
  fields.push_back(nodesP2.uri());
  boost::shared_ptr< MeshWriter > tec_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.tecplot.Writer","meshwriter");
  tec_writer->options().set("cell_centred",true);
  tec_writer->options().set("mesh",mesh.handle<Mesh const>());
  tec_writer->options().set("fields",fields);
  tec_writer->options().set("file",URI("quadtriag.plt"));
  tec_writer->execute();

  std::vector<URI> regions;
  regions.push_back(mesh.uri()/"topology/inlet");
  regions.push_back(mesh.uri()/"topology/outlet");
  regions.push_back(mesh.uri()/"topology/wall");
  regions.push_back(mesh.uri()/"topology/liquid");
  tec_writer->options().set("regions",regions);
  tec_writer->options().set("file",URI("quadtriag_filtered.plt"));
  tec_writer->execute();

  BOOST_CHECK(true);
}

////////////////////////////////////////////////////////////////////////////////
/*
BOOST_AUTO_TEST_CASE( threeD_test )
{

  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");

  meshreader->options().set("number_of_processors",(Uint) Comm::instance().size());
  meshreader->options().set("rank",(Uint) Comm::instance().rank());
  meshreader->options().set("Repartition",false);
  meshreader->options().set("OutputRank",(Uint) 2);

  // the file to read from
  boost::filesystem::path fp_in ("../../resources/hextet.neu");

  // the mesh to store in
  boost::shared_ptr< Mesh > mesh ( allocate_component<Mesh>  ( "mesh" ) );


  CFinfo.setFilterRankZero(false);
  meshreader->do_read_mesh_into(fp_in,mesh);
  CFinfo.setFilterRankZero(true);

  boost::filesystem::path fp_out ("hextet.msh");
  boost::shared_ptr< MeshWriter > gmsh_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  gmsh_writer->write_from_to(mesh,fp_out);

  BOOST_CHECK(true);

}
*/
////////////////////////////////////////////////////////////////////////////////
/*
BOOST_AUTO_TEST_CASE( read_multiple_2D )
{

  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");

  meshreader->options().set("Repartition",true);
  meshreader->options().set("OutputRank",(Uint) 0);

  // the file to read from
  boost::filesystem::path fp_in ("quadtriag.neu");

  // the mesh to store in
  boost::shared_ptr< Mesh > mesh ( allocate_component<Mesh>  ( "mesh" ) );


  CFinfo.setFilterRankZero(false);



  for (Uint count=1; count<=2; ++count)
  {
    CFinfo << "\n\n\nMesh parallel:" << CFendl;
    meshreader->do_read_mesh_into(fp_in,mesh);
  }



  CFinfo.setFilterRankZero(true);
  CFinfo << mesh->tree() << CFendl;
  CFinfo << meshreader->tree() << CFendl;
  boost::shared_ptr< MeshTransformer > info  = build_component_abstract_type<MeshTransformer>("Info","info");
  info->transform(mesh);


  boost::filesystem::path fp_out ("quadtriag_mult.msh");
  boost::shared_ptr< MeshWriter > gmsh_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  gmsh_writer->write_from_to(mesh,fp_out);

  BOOST_CHECK_EQUAL(1,1);

}
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

