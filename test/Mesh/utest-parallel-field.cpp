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
#include "common/Core.hpp"
#include "common/CRoot.hpp"
#include "common/CEnv.hpp"

#include "common/Foreach.hpp"
#include "common/OSystem.hpp"
#include "common/OSystemLayer.hpp"

#include "common/PE/CommPattern.hpp"
#include "common/PE/CommWrapperMArray.hpp"
#include "common/PE/debug.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshGenerator.hpp"
#include "Mesh/CMeshPartitioner.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/CSpace.hpp"

#include "Tools/Gnuplot/Gnuplot.hpp"

using namespace boost;
using namespace cf3;
using namespace cf3::Mesh;
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
  Core::instance().environment().configure_option("log_level",(Uint)DEBUG);

  // Create or read the mesh

#define GEN

#ifdef GEN
  CMeshGenerator::Ptr meshgenerator = build_component_abstract_type<CMeshGenerator>("CF.Mesh.CSimpleMeshGenerator","1Dgenerator");
  meshgenerator->configure_option("mesh",URI("//Root/rect"));
  std::vector<Uint> nb_cells(2);
  std::vector<Real> lengths(2);
  nb_cells[0] = 10;
  nb_cells[1] = 5;
  lengths[0]  = nb_cells[0];
  lengths[1]  = nb_cells[1];
  meshgenerator->configure_option("nb_cells",nb_cells);
  meshgenerator->configure_option("lengths",lengths);
  meshgenerator->configure_option("bdry",false);
  CMesh& mesh = meshgenerator->generate();
#endif

#ifdef NEU
  CMeshReader::Ptr meshreader =
      build_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
  CMesh::Ptr mesh_ptr = meshreader->create_mesh_from("rotation-tg-p1.neu");
  CMesh& mesh = *mesh_ptr;
#endif

#ifdef GMSH
  CMeshReader::Ptr meshreader =
      build_component_abstract_type<CMeshReader>("CF.Mesh.Gmsh.CReader","meshreader");
  CMesh::Ptr mesh_ptr = meshreader->create_mesh_from("rectangle-tg-p1.msh");
  CMesh& mesh = *mesh_ptr;
#endif


  Core::instance().root().add_component(mesh);

  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.LoadBalance","load_balancer")->transform(mesh);

 // Core::instance().tools().get_child("LoadBalancer").as_type<CMeshTransformer>().transform(mesh);

  // create a field and assign it to the comm pattern

  Field& field = mesh.geometry().create_field("node_rank");

  field.parallelize();

  for (Uint n=0; n<field.size(); ++n)
    for (Uint j=0; j<field.row_size(); ++j)
      field[n][j] = PE::Comm::instance().rank();

  // Synchronize

  // comm_pattern.synchronize(); // via the comm_pattern

  field.synchronize(); // via the field


  BOOST_CHECK(true); // Tadaa

  // Create a field with glb element numbers
  FieldGroup& elems_P0 = mesh.create_space_and_field_group("elems_P0",FieldGroup::Basis::ELEMENT_BASED,"CF.Mesh.LagrangeP0");
  Field& glb_elem_idx  = elems_P0.create_field("glb_elem");
  Field& elem_rank     = elems_P0.create_field("elem_rank");


  FieldGroup& nodes_P1 = mesh.create_space_and_field_group("nodes_P1",FieldGroup::Basis::POINT_BASED,"CF.Mesh.LagrangeP2");
  Field& nodes_P1_node_rank = nodes_P1.create_field("node_rank");
  nodes_P1_node_rank.parallelize();
  for (Uint n=0; n<nodes_P1_node_rank.size(); ++n)
    nodes_P1_node_rank[n][0] = nodes_P1.rank()[n];

  boost_foreach(CElements& elements , elems_P0.elements_range())
  {
    CSpace& space = elems_P0.space(elements);
    for (Uint elem=0; elem<elements.size(); ++elem)
    {
      Uint field_idx = space.indexes_for_element(elem)[0];
      glb_elem_idx[field_idx][0] = elems_P0.glb_idx()[field_idx];
      elem_rank[field_idx][0] = elems_P0.rank()[field_idx];
    }
  }

  // Create a field with glb node numbers
  Field& glb_node_idx = mesh.geometry().create_field("glb_node_idx");

  CList<Uint>& glb_idx = mesh.geometry().glb_idx();
  {
    for (Uint n=0; n<glb_node_idx.size(); ++n)
      glb_node_idx[n][0] = glb_idx[n];
  }

  // Create a field with glb node numbers
  Field& P1_node_rank = mesh.geometry().create_field("P1_node_rank");

  CAction& interpolator = mesh.create_component("interpolator","CF.Mesh.Actions.Interpolate").as_type<CAction>();
  interpolator.configure_option("source",nodes_P1_node_rank.uri());
  interpolator.configure_option("target",P1_node_rank.uri());
  interpolator.execute();

  // Write the mesh with the fields

  std::vector<Field::Ptr> fields;
  fields.push_back(field.as_ptr<Field>());
  fields.push_back(P1_node_rank.as_ptr<Field>());
  fields.push_back(glb_elem_idx.as_ptr<Field>());
  fields.push_back(elem_rank.as_ptr<Field>());
  fields.push_back(glb_node_idx.as_ptr<Field>());

  CMeshWriter::Ptr tec_writer =
      build_component_abstract_type<CMeshWriter>("CF.Mesh.Tecplot.CWriter","tec_writer");

  tec_writer->set_fields(fields);
  tec_writer->configure_option("cell_centred",true);
  tec_writer->write_from_to(mesh,"parallel_fields.plt");

  CFinfo << "parallel_fields_P*.plt written" << CFendl;

  CMeshWriter::Ptr msh_writer =
      build_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","msh_writer");

  msh_writer->set_fields(fields);
  msh_writer->write_from_to(mesh,"parallel_fields.msh");

  CFinfo << "parallel_fields_P*.msh written" << CFendl;


}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( minitest )
{
  CFinfo << "ParallelFields_test" << CFendl;
  Core::instance().environment().configure_option("log_level",(Uint)DEBUG);

  CMeshGenerator::Ptr meshgenerator = build_component_abstract_type<CMeshGenerator>("CF.Mesh.CSimpleMeshGenerator","1Dgenerator");
  meshgenerator->configure_option("mesh",URI("//Root/line"));
  meshgenerator->configure_option("nb_cells",std::vector<Uint>(1,10));
  meshgenerator->configure_option("lengths",std::vector<Real>(1,10.));
  meshgenerator->configure_option("bdry",false);
  CMesh& mesh = meshgenerator->generate();

  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.LoadBalance","load_balancer")->transform(mesh);

  // create a field and assign it to the comm pattern

  Field& node_rank = mesh.geometry().create_field("node_rank");
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
        BOOST_CHECK_EQUAL( mesh.geometry().coordinates()[0][XX] , 0.);
        BOOST_CHECK_EQUAL( node_rank[0][0] , 0. );
        BOOST_CHECK_EQUAL( mesh.geometry().coordinates()[1][XX] , 1.);
        BOOST_CHECK_EQUAL( node_rank[1][0] , 0. );
        BOOST_CHECK_EQUAL( mesh.geometry().coordinates()[2][XX] , 2.);
        BOOST_CHECK_EQUAL( node_rank[2][0] , 0. );
        BOOST_CHECK_EQUAL( mesh.geometry().coordinates()[3][XX] , 3.);
        BOOST_CHECK_EQUAL( node_rank[3][0] , 0. );
        BOOST_CHECK_EQUAL( mesh.geometry().coordinates()[4][XX] , 4.);
        BOOST_CHECK_EQUAL( node_rank[4][0] , 0. );

        BOOST_CHECK_EQUAL( mesh.geometry().coordinates()[5][XX] , 5.);
        BOOST_CHECK_EQUAL( node_rank[5][0] , 1. );
        BOOST_CHECK_EQUAL( mesh.geometry().coordinates()[6][XX] , 6.);
        BOOST_CHECK_EQUAL( node_rank[6][0] , 1. );
      }
      else if (Comm::instance().rank() == 1)
      {
        BOOST_CHECK_EQUAL( mesh.geometry().coordinates()[0][XX] , 5.);
        BOOST_CHECK_EQUAL( node_rank[0][0] , 1. );
        BOOST_CHECK_EQUAL( mesh.geometry().coordinates()[1][XX] , 6.);
        BOOST_CHECK_EQUAL( node_rank[1][0] , 1. );
        BOOST_CHECK_EQUAL( mesh.geometry().coordinates()[2][XX] , 7.);
        BOOST_CHECK_EQUAL( node_rank[2][0] , 1. );
        BOOST_CHECK_EQUAL( mesh.geometry().coordinates()[3][XX] , 8.);
        BOOST_CHECK_EQUAL( node_rank[3][0] , 1. );
        BOOST_CHECK_EQUAL( mesh.geometry().coordinates()[4][XX] , 9.);
        BOOST_CHECK_EQUAL( node_rank[4][0] , 1. );
        BOOST_CHECK_EQUAL( mesh.geometry().coordinates()[5][XX] , 10.);
        BOOST_CHECK_EQUAL( node_rank[5][0] , 1. );

        BOOST_CHECK_EQUAL( mesh.geometry().coordinates()[6][XX] , 4.);
        BOOST_CHECK_EQUAL( node_rank[6][0] , 0. );
      }
    }
    else
      CFwarn << "Checks only performed when run with 2 procs" << CFendl;
  }


  FieldGroup& elems = mesh.create_space_and_field_group("elems_P0",FieldGroup::Basis::ELEMENT_BASED,"CF.Mesh.LagrangeP0");
  elems.create_coordinates();
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
  gp.send( mesh.geometry().coordinates().array()  , node_rank.array() );
  gp.send( elems.coordinates().array()            , elem_rank.array() );

//   Following works too, if coordinates need to be in sequence
//    std::map<Real,Real> nodes_xy;
//    for (Uint i=0; i<node_rank.size(); ++i)
//      nodes_xy[mesh.geometry().coordinates()[i][XX]] = node_rank[i][0];

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

