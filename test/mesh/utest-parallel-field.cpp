// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for parallel fields"

#include <iomanip>
#include <boost/test/unit_test.hpp>

#include "coolfluid-packages.hpp"

#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/Core.hpp"
#include "common/Environment.hpp"

#include "common/Foreach.hpp"
#include "common/OSystem.hpp"
#include "common/OSystemLayer.hpp"

#include "common/PE/CommPattern.hpp"
#include "common/PE/CommWrapperMArray.hpp"
#include "common/PE/debug.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Region.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/MeshGenerator.hpp"
#include "mesh/MeshPartitioner.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/Space.hpp"

#include "Tools/Gnuplot/Gnuplot.hpp"

using namespace boost;
using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::common;
using namespace cf3::common::PE;

////////////////////////////////////////////////////////////////////////////////

struct ParallelFieldsTests_Fixture
{
  /// common setup for each test case
  ParallelFieldsTests_Fixture()
  {
    // uncomment if you want to use arguments to the test executable
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;

  }

  /// common tear-down for each test case
  ~ParallelFieldsTests_Fixture()
  {

  }

  /// possibly common functions used on the tests below

  int m_argc;
  char** m_argv;
};


////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( ParallelFieldsTests_TestSuite, ParallelFieldsTests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
  Core::instance().initiate(m_argc,m_argv);
  PE::Comm::instance().init(m_argc,m_argv);

}
 ////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( parallelize_and_synchronize )
{
  CFinfo << "ParallelFields_test" << CFendl;
  Core::instance().environment().options().configure_option("log_level",(Uint)DEBUG);

  // Create or read the mesh

#define GEN

#ifdef GEN
  boost::shared_ptr< MeshGenerator > meshgenerator = build_component_abstract_type<MeshGenerator>("cf3.mesh.SimpleMeshGenerator","1Dgenerator");
  meshgenerator->options().configure_option("mesh",URI("//rect"));
  std::vector<Uint> nb_cells(2);
  std::vector<Real> lengths(2);
  nb_cells[0] = 10;
  nb_cells[1] = 5;
  lengths[0]  = nb_cells[0];
  lengths[1]  = nb_cells[1];
  meshgenerator->options().configure_option("nb_cells",nb_cells);
  meshgenerator->options().configure_option("lengths",lengths);
  meshgenerator->options().configure_option("bdry",false);
  Mesh& mesh = meshgenerator->generate();
#endif

#ifdef NEU
  Handle< MeshReader > meshreader =
      build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");
  Handle< Mesh > mesh_ptr = meshreader->create_mesh_from("rotation-tg-p1.neu");
  Mesh& mesh = *mesh_ptr;
  Core::instance().root().add_component(mesh_ptr):
#endif

#ifdef GMSH
  Handle< MeshReader > meshreader =
      build_component_abstract_type<MeshReader>("cf3.mesh.gmsh.Reader","meshreader");
  Handle< Mesh > mesh_ptr = meshreader->create_mesh_from("rectangle-tg-p1.msh");
  Mesh& mesh = *mesh_ptr;
  Core::instance().root().add_component(mesh_ptr):
#endif

build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.LoadBalance","load_balancer")->transform(mesh);

 // Core::instance().tools().get_child("LoadBalancer").as_type<MeshTransformer>().transform(mesh);

  // create a field and assign it to the comm pattern

  Field& field = mesh.geometry_fields().create_field("node_rank");

  field.parallelize();

  for (Uint n=0; n<field.size(); ++n)
    for (Uint j=0; j<field.row_size(); ++j)
      field[n][j] = PE::Comm::instance().rank();

  // Synchronize

  // comm_pattern.synchronize(); // via the comm_pattern

  field.synchronize(); // via the field


  BOOST_CHECK(true); // Tadaa

  // Create a field with glb element numbers
  Dictionary& elems_P0 = mesh.create_discontinuous_space("elems_P0","cf3.mesh.LagrangeP0");
  Field& glb_elem_idx  = elems_P0.create_field("glb_elem");
  Field& elem_rank     = elems_P0.create_field("elem_rank");


  Dictionary& nodes_P1 = mesh.create_continuous_space("nodes_P1","cf3.mesh.LagrangeP2");
  Field& nodes_P1_node_rank = nodes_P1.create_field("node_rank");
  nodes_P1_node_rank.parallelize();
  for (Uint n=0; n<nodes_P1_node_rank.size(); ++n)
    nodes_P1_node_rank[n][0] = nodes_P1.rank()[n];

  boost_foreach(const Handle<Entities>& elements_handle, elems_P0.entities_range())
  {
    const Entities& elements = *elements_handle;
    const Space& space = elems_P0.space(elements);
    for (Uint elem=0; elem<elements.size(); ++elem)
    {
      Uint field_idx = space.connectivity()[elem][0];
      glb_elem_idx[field_idx][0] = elems_P0.glb_idx()[field_idx];
      elem_rank[field_idx][0] = elems_P0.rank()[field_idx];
    }
  }

  // Create a field with glb node numbers
  Field& glb_node_idx = mesh.geometry_fields().create_field("glb_node_idx");

  List<Uint>& glb_idx = mesh.geometry_fields().glb_idx();
  {
    for (Uint n=0; n<glb_node_idx.size(); ++n)
      glb_node_idx[n][0] = glb_idx[n];
  }

  // Create a field with glb node numbers
  Field& P1_node_rank = mesh.geometry_fields().create_field("P1_node_rank");

  Handle<Action> interpolator(mesh.create_component("interpolator","cf3.mesh.actions.Interpolate"));
  interpolator->options().configure_option("source",nodes_P1_node_rank.handle<Field const>());
  interpolator->options().configure_option("target",P1_node_rank.handle<Field>());
  interpolator->execute();

  // Write the mesh with the fields

  std::vector<URI> fields;
  fields.push_back(field.uri());
  fields.push_back(P1_node_rank.uri());
  fields.push_back(glb_elem_idx.uri());
  fields.push_back(elem_rank.uri());
  fields.push_back(glb_node_idx.uri());

  boost::shared_ptr< MeshWriter > tec_writer =
      build_component_abstract_type<MeshWriter>("cf3.mesh.tecplot.Writer","tec_writer");

  tec_writer->options().configure_option("fields",fields);
  tec_writer->options().configure_option("cell_centred",true);
  tec_writer->options().configure_option("file",URI("parallel_fields.plt"));
  tec_writer->options().configure_option("mesh",mesh.handle<Mesh>());
  tec_writer->execute();

  CFinfo << "parallel_fields_P*.plt written" << CFendl;

  boost::shared_ptr< MeshWriter > msh_writer =
      build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","msh_writer");

  msh_writer->options().configure_option("fields",fields);
  msh_writer->options().configure_option("file",URI("parallel_fields.msh"));
  msh_writer->options().configure_option("mesh",mesh.handle<Mesh>());
  msh_writer->execute();

  CFinfo << "parallel_fields_P*.msh written" << CFendl;


}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( minitest )
{
  CFinfo << "ParallelFields_test" << CFendl;
  Core::instance().environment().options().configure_option("log_level",(Uint)DEBUG);

  boost::shared_ptr< MeshGenerator > meshgenerator = build_component_abstract_type<MeshGenerator>("cf3.mesh.SimpleMeshGenerator","1Dgenerator");
  meshgenerator->options().configure_option("mesh",URI("//line"));
  meshgenerator->options().configure_option("nb_cells",std::vector<Uint>(1,10));
  meshgenerator->options().configure_option("lengths",std::vector<Real>(1,10.));
  meshgenerator->options().configure_option("bdry",false);
  Mesh& mesh = meshgenerator->generate();

  build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.LoadBalance","load_balancer")->transform(mesh);

  // create a field and assign it to the comm pattern

  Field& node_rank = mesh.geometry_fields().create_field("node_rank");
  node_rank.parallelize();


  CFinfo << "Node test" << CFendl;
  for (Uint n=0; n<node_rank.size(); ++n)
    for (Uint j=0; j<node_rank.row_size(); ++j)
      node_rank[n][j] = PE::Comm::instance().rank();
  node_rank.synchronize();

  if (0)
  {
    if (Comm::instance().size() == 2)
    {
      if (Comm::instance().rank() == 0)
      {
        BOOST_CHECK_EQUAL( mesh.geometry_fields().coordinates()[0][XX] , 0.);
        BOOST_CHECK_EQUAL( node_rank[0][0] , 0. );
        BOOST_CHECK_EQUAL( mesh.geometry_fields().coordinates()[1][XX] , 1.);
        BOOST_CHECK_EQUAL( node_rank[1][0] , 0. );
        BOOST_CHECK_EQUAL( mesh.geometry_fields().coordinates()[2][XX] , 2.);
        BOOST_CHECK_EQUAL( node_rank[2][0] , 0. );
        BOOST_CHECK_EQUAL( mesh.geometry_fields().coordinates()[3][XX] , 3.);
        BOOST_CHECK_EQUAL( node_rank[3][0] , 0. );
        BOOST_CHECK_EQUAL( mesh.geometry_fields().coordinates()[4][XX] , 4.);
        BOOST_CHECK_EQUAL( node_rank[4][0] , 0. );

        BOOST_CHECK_EQUAL( mesh.geometry_fields().coordinates()[5][XX] , 5.);
        BOOST_CHECK_EQUAL( node_rank[5][0] , 1. );
        BOOST_CHECK_EQUAL( mesh.geometry_fields().coordinates()[6][XX] , 6.);
        BOOST_CHECK_EQUAL( node_rank[6][0] , 1. );
      }
      else if (Comm::instance().rank() == 1)
      {
        BOOST_CHECK_EQUAL( mesh.geometry_fields().coordinates()[0][XX] , 5.);
        BOOST_CHECK_EQUAL( node_rank[0][0] , 1. );
        BOOST_CHECK_EQUAL( mesh.geometry_fields().coordinates()[1][XX] , 6.);
        BOOST_CHECK_EQUAL( node_rank[1][0] , 1. );
        BOOST_CHECK_EQUAL( mesh.geometry_fields().coordinates()[2][XX] , 7.);
        BOOST_CHECK_EQUAL( node_rank[2][0] , 1. );
        BOOST_CHECK_EQUAL( mesh.geometry_fields().coordinates()[3][XX] , 8.);
        BOOST_CHECK_EQUAL( node_rank[3][0] , 1. );
        BOOST_CHECK_EQUAL( mesh.geometry_fields().coordinates()[4][XX] , 9.);
        BOOST_CHECK_EQUAL( node_rank[4][0] , 1. );
        BOOST_CHECK_EQUAL( mesh.geometry_fields().coordinates()[5][XX] , 10.);
        BOOST_CHECK_EQUAL( node_rank[5][0] , 1. );

        BOOST_CHECK_EQUAL( mesh.geometry_fields().coordinates()[6][XX] , 4.);
        BOOST_CHECK_EQUAL( node_rank[6][0] , 0. );
      }
    }
    else
      CFwarn << "Checks only performed when run with 2 procs" << CFendl;
  }


  Dictionary& elems = mesh.create_discontinuous_space("elems_P0","cf3.mesh.LagrangeP0");
  Field& elem_rank     = elems.create_field("elem_rank");
  elem_rank.parallelize();

  CFinfo << "Elem test" << CFendl;
   for (Uint n=0; n<elem_rank.size(); ++n)
     for (Uint j=0; j<elem_rank.row_size(); ++j)
       elem_rank[n][j] = PE::Comm::instance().rank();
   elem_rank.synchronize();

  if (0)
  {
    if (Comm::instance().size() == 2)
    {
      if (Comm::instance().rank() == 0)
      {
        BOOST_CHECK_EQUAL( elems.coordinates()[0][XX] , 0.5 );
        BOOST_CHECK_EQUAL( elem_rank[0][0] , 0 );
        BOOST_CHECK_EQUAL( elems.coordinates()[1][XX] , 1.5 );
        BOOST_CHECK_EQUAL( elem_rank[1][0] , 0 );
        BOOST_CHECK_EQUAL( elems.coordinates()[2][XX] , 2.5 );
        BOOST_CHECK_EQUAL( elem_rank[2][0] , 0 );
        BOOST_CHECK_EQUAL( elems.coordinates()[3][XX] , 3.5 );
        BOOST_CHECK_EQUAL( elem_rank[3][0] , 0 );
        BOOST_CHECK_EQUAL( elems.coordinates()[4][XX] , 4.5 );
        BOOST_CHECK_EQUAL( elem_rank[4][0] , 0 );
        BOOST_CHECK_EQUAL( elems.coordinates()[5][XX] , 5.5 );
        BOOST_CHECK_EQUAL( elem_rank[5][0] , 1 );
      }
      else if (Comm::instance().rank() == 1)
      {
        BOOST_CHECK_EQUAL( elems.coordinates()[0][XX] , 5.5 );
        BOOST_CHECK_EQUAL( elem_rank[0][0] , 1 );
        BOOST_CHECK_EQUAL( elems.coordinates()[1][XX] , 6.5 );
        BOOST_CHECK_EQUAL( elem_rank[1][0] , 1 );
        BOOST_CHECK_EQUAL( elems.coordinates()[2][XX] , 7.5 );
        BOOST_CHECK_EQUAL( elem_rank[2][0] , 1 );
        BOOST_CHECK_EQUAL( elems.coordinates()[3][XX] , 8.5 );
        BOOST_CHECK_EQUAL( elem_rank[3][0] , 1 );
        BOOST_CHECK_EQUAL( elems.coordinates()[4][XX] , 9.5 );
        BOOST_CHECK_EQUAL( elem_rank[4][0] , 1 );
        BOOST_CHECK_EQUAL( elems.coordinates()[5][XX] , 4.5 );
        BOOST_CHECK_EQUAL( elem_rank[5][0] , 0 );
      }
    }
  }

#ifdef GNUPLOT_FOUND
//  Gnuplot gp(std::string(GNUPLOT_COMMAND)+std::string(" -persist"));
  Gnuplot gp(std::string(GNUPLOT_COMMAND));
  gp << "set terminal png" << std::endl;
  gp << "set output 'ranks_P"<<Comm::instance().rank()<<".png'" << std::endl;
  gp << "set yrange [-0.5:1.5]" << std::endl;
  gp << "set title 'Rank "<<Comm::instance().rank()<<"'" << std::endl;
  gp << "plot ";
  gp << "'-' with points title 'node-rank'"  << ", ";
  gp << "'-' with points title 'elem-rank'"  << "\n";
  gp.send( mesh.geometry_fields().coordinates().array()  , node_rank.array() );
  gp.send( elems.coordinates().array()            , elem_rank.array() );

//   Following works too, if coordinates need to be in sequence
//    std::map<Real,Real> nodes_xy;
//    for (Uint i=0; i<node_rank.size(); ++i)
//      nodes_xy[mesh.geometry_fields().coordinates()[i][XX]] = node_rank[i][0];

//    std::map<Real,Real> elems_xy;
//    for (Uint i=0; i<elem_rank.size(); ++i)
//      elems_xy[elems.coordinates()[i][XX]] = elem_rank[i][0];

//    gp.send(nodes_xy);
//    gp.send(elems_xy);
#endif
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
  PE::Comm::instance().finalize();

  Core::instance().terminate();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

