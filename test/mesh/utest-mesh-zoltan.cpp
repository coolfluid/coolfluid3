// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for zoltan load balancing library"

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "common/Foreach.hpp"
#include "common/OSystem.hpp"
#include "common/OSystemLayer.hpp"

#include "common/PE/Comm.hpp"
#include "common/PE/debug.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Region.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/MeshGenerator.hpp"
#include "mesh/MeshPartitioner.hpp"
#include "mesh/MeshTransformer.hpp"

using namespace boost;
using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::common;
using namespace cf3::common::PE;

////////////////////////////////////////////////////////////////////////////////

struct ZoltanTests_Fixture
{
  /// common setup for each test case
  ZoltanTests_Fixture()
  {
    // uncomment if you want to use arguments to the test executable
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;

  }

  /// common tear-down for each test case
  ~ZoltanTests_Fixture()
  {

  }

  /// possibly common functions used on the tests below

  int m_argc;
  char** m_argv;
};


////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( ZoltanTests_TestSuite, ZoltanTests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
  Core::instance().initiate(m_argc,m_argv);
  PE::Comm::instance().init(m_argc,m_argv);

}

////////////////////////////////////////////////////////////////////////////////
/*
BOOST_AUTO_TEST_CASE( MeshPartitioner_test_quadtriag )
{
  Core::instance().environment().options().configure_option("log_level",(Uint)DEBUG);
  CFinfo << "MeshPartitioner_test" << CFendl;
  boost::shared_ptr< MeshReader > meshreader = build_component_abstract_type<MeshReader>("cf3.mesh.neu.Reader","meshreader");
  meshreader->options().configure_option("read_boundaries",false);

  // the file to read from
  URI fp_in ("../../resources/quadtriag.neu");

  // the mesh to store in
  Handle< Mesh > mesh_ptr = meshreader->create_mesh_from(fp_in);
  Mesh& mesh = *mesh_ptr;

  boost::shared_ptr< MeshTransformer > glb_numbering = build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.GlobalNumbering","glb_numbering");
  glb_numbering->transform(mesh_ptr);
  boost::shared_ptr< MeshTransformer > glb_connectivity = build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.GlobalConnectivity","glb_connectivity");
  glb_connectivity->transform(mesh_ptr);

  boost::shared_ptr< MeshWriter > meshwriter = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  URI fp_out_1 ("quadtriag.msh");
  meshwriter->write_from_to(*mesh_ptr,fp_out_1);

  boost::shared_ptr< MeshPartitioner > partitioner_ptr = build_component_abstract_type<MeshTransformer>("cf3.mesh.zoltan.Partitioner","partitioner")->as_ptr<MeshPartitioner>();

  MeshPartitioner& p = *partitioner_ptr;
  BOOST_CHECK_EQUAL(p.name(),"partitioner");

  Core::instance().initiate(m_argc,m_argv);

  //p.options().configure_option("nb_parts", (Uint) 4);
  p.options().configure_option("graph_package", std::string("PHG"));
  p.options().configure_option("debug_level", 2u);
  BOOST_CHECK(true);
  p.initialize(mesh);

  BOOST_CHECK_EQUAL(p.proc_of_obj(0), 0u);
  BOOST_CHECK_EQUAL(p.proc_of_obj(7), 0u);
  BOOST_CHECK_EQUAL(p.proc_of_obj(8), 0u);
  BOOST_CHECK_EQUAL(p.proc_of_obj(15), 0u);
  BOOST_CHECK_EQUAL(p.proc_of_obj(16), 1u);
  BOOST_CHECK_EQUAL(p.proc_of_obj(23), 1u);
  BOOST_CHECK_EQUAL(p.proc_of_obj(24), 1u);
  BOOST_CHECK_EQUAL(p.proc_of_obj(31), 1u);

  BOOST_CHECK_EQUAL(p.is_node(0), true);
  BOOST_CHECK_EQUAL(p.is_node(7), true);
  BOOST_CHECK_EQUAL(p.is_node(8), false);
  BOOST_CHECK_EQUAL(p.is_node(15), false);
  BOOST_CHECK_EQUAL(p.is_node(16), true);
  BOOST_CHECK_EQUAL(p.is_node(23), true);
  BOOST_CHECK_EQUAL(p.is_node(24), false);
  BOOST_CHECK_EQUAL(p.is_node(31), false);

  Uint comp_idx;
  Handle< Component > comp;
  Uint idx;
  bool found;
  if ( PE::Comm::instance().rank() == 0)
  {
    boost::tie(comp,idx) = p.to_local(0);
    boost::tie(comp_idx,idx,found) = p.to_local_indices_from_glb_obj(0);
    BOOST_CHECK( is_not_null(comp->as_ptr<Dictionary>()) );
    BOOST_CHECK_EQUAL(comp_idx, 0);
    BOOST_CHECK_EQUAL(idx, 0);
    BOOST_CHECK_EQUAL(found, true);

    boost::tie(comp_idx,idx,found) = p.to_local_indices_from_glb_obj(7);
    BOOST_CHECK_EQUAL(comp_idx, 0);
    BOOST_CHECK_EQUAL(idx, 7);
    BOOST_CHECK_EQUAL(found, true);

  }

  BOOST_CHECK(true);
  p.partition_graph();
  BOOST_CHECK(true);
  p.show_changes();
  BOOST_CHECK(true);
  p.migrate();
  BOOST_CHECK(true);

  boost::shared_ptr< MeshTransformer > glb_node_numbering = build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.GlobalNumberingNodes","glb_node_numbering");
  glb_node_numbering->options().configure_option("debug",true);
  glb_node_numbering->transform(mesh);


  PEProcessSortedExecute(-1,
      std::cout << "rank  = "  << Comm::instance().rank() << std::endl;
      std::cout << "nodes = " << mesh.geometry_fields().glb_idx() << std::endl;
      std::cout << "ranks = " << mesh.geometry_fields().rank() << std::endl;
      boost_foreach(const Entities& entities, mesh.topology().elements_range())
      {
        //std::cout << "elems = " << entities.glb_idx() << std::endl;
      }

  )

  URI fp_out_2 ("quadtriag_repartitioned.msh");
  meshwriter->write_from_to(*mesh_ptr,fp_out_2);
}
*/
BOOST_AUTO_TEST_CASE( MeshPartitioner_test_quadtriag )
{
  Core::instance().environment().options().configure_option("log_level",(Uint)DEBUG);
  boost::shared_ptr< MeshGenerator > meshgenerator = build_component_abstract_type<MeshGenerator>("cf3.mesh.SimpleMeshGenerator","1Dgenerator");

  meshgenerator->options().configure_option("mesh",URI("//rect"));
  std::vector<Uint> nb_cells(2);  nb_cells[0] = 3;   nb_cells[1] = 2;
  std::vector<Real> lengths(2);   lengths[0]  = nb_cells[0];  lengths[1]  = nb_cells[1];
  meshgenerator->options().configure_option("nb_cells",nb_cells);
  meshgenerator->options().configure_option("lengths",lengths);
  meshgenerator->options().configure_option("bdry",false);
  Mesh& mesh = meshgenerator->generate();


  boost::shared_ptr< MeshTransformer > glb_numbering = build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.GlobalNumbering","glb_numbering");
  glb_numbering->transform(mesh);
  boost::shared_ptr< MeshTransformer > glb_connectivity = build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.GlobalConnectivity","glb_connectivity");
  glb_connectivity->transform(mesh);

  boost::shared_ptr< MeshWriter > meshwriter = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  meshwriter->write_from_to(mesh,"rect.msh");

  boost::shared_ptr< MeshPartitioner > partitioner_ptr = boost::dynamic_pointer_cast<MeshPartitioner>(build_component_abstract_type<MeshTransformer>("cf3.mesh.zoltan.Partitioner","partitioner"));

  MeshPartitioner& p = *partitioner_ptr;
  BOOST_CHECK_EQUAL(p.name(),"partitioner");

  Core::instance().initiate(m_argc,m_argv);

  //p.options().configure_option("nb_parts", (Uint) 4);
  p.options().configure_option("graph_package", std::string("PHG"));
  p.options().configure_option("debug_level", 2u);
  BOOST_CHECK(true);
  p.initialize(mesh);
  BOOST_CHECK(true);
  p.partition_graph();
  BOOST_CHECK(true);
  p.show_changes();
  BOOST_CHECK(true);
  p.migrate();
  BOOST_CHECK(true);

  boost::shared_ptr< MeshTransformer > glb_node_numbering = build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.GlobalNumberingNodes","glb_node_numbering");
  glb_node_numbering->options().configure_option("debug",true);
  glb_node_numbering->transform(mesh);


  PEProcessSortedExecute(-1,
      std::cout << PERank << "nodes = " << mesh.geometry_fields().coordinates() << std::endl;
      std::cout << PERank << "ranks = " << mesh.geometry_fields().rank() << std::endl;
      boost_foreach(const Entities& entities, mesh.topology().elements_range())
      {
        //std::cout << "elems = " << entities.glb_idx() << std::endl;
      }

  )


  boost::shared_ptr< MeshWriter > tecwriter = build_component_abstract_type<MeshWriter>("cf3.mesh.tecplot.Writer","meshwriter");
  tecwriter->write_from_to(mesh,"rect_repartitioned.plt");
  meshwriter->write_from_to(mesh,"rect_repartitioned.msh");

  CFinfo << "zoltan version:" << p.properties().value<Real>("Zoltan_version") << CFendl;

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

