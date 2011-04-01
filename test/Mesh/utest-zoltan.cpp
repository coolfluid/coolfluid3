// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for Zoltan load balancing library"

// boost
#include <boost/test/unit_test.hpp>

// coolfluid
#include "Common/Log.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/Foreach.hpp"
#include "Common/OSystem.hpp"
#include "Common/OSystemLayer.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshPartitioner.hpp"
#include "Mesh/CMeshTransformer.hpp"

using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;

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
  mpi::PE::instance().init(m_argc,m_argv);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( CMeshPartitioner_test )
{
  CFinfo << "CMeshPartitioner_test" << CFendl;
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
  meshreader->configure_property("read_boundaries",false);

  // the file to read from
  boost::filesystem::path fp_in ("quadtriag.neu");

  // the mesh to store in
  CMesh::Ptr mesh_ptr = meshreader->create_mesh_from(fp_in);
  CMesh& mesh = *mesh_ptr;
  
  CMeshTransformer::Ptr glb_numbering = create_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumbering","glb_numbering");
  glb_numbering->transform(mesh_ptr);
  CMeshTransformer::Ptr glb_connectivity = create_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalConnectivity","glb_connectivity");
  glb_connectivity->transform(mesh_ptr);

  CMeshWriter::Ptr meshwriter = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  boost::filesystem::path fp_out_1 ("quadtriag.msh");
  meshwriter->write_from_to(mesh_ptr,fp_out_1);

  CMeshPartitioner::Ptr partitioner_ptr = create_component_abstract_type<CMeshPartitioner>("CF.Mesh.Zoltan.CPartitioner","partitioner");

  CMeshPartitioner& p = *partitioner_ptr;
  BOOST_CHECK_EQUAL(p.name(),"partitioner");

  Core::instance().initiate(m_argc,m_argv);

  //p.configure_property("Number of Partitions", (Uint) 4);
  p.configure_property("graph_package", std::string("PHG"));
  p.configure_property("debug_level", 2u);
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
  Component::Ptr comp;
  Uint idx;
  bool found;
  if ( mpi::PE::instance().rank() == 0)
  {
    boost::tie(comp,idx) = p.to_local(0);
    boost::tie(comp_idx,idx,found) = p.to_local_indices_from_glb_obj(0);
    BOOST_CHECK( is_not_null(comp->as_ptr<CNodes>()) );
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
  boost::filesystem::path fp_out_2 ("quadtriag_repartitioned.msh");
  meshwriter->write_from_to(mesh_ptr,fp_out_2);
}

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
  mpi::PE::instance().finalize();

  Core::instance().terminate();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

