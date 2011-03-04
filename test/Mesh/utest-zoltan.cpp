// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for Zoltan load balancing library"

// boost
#include <boost/test/unit_test.hpp>
#include <boost/filesystem/path.hpp>

// coolfluid
#include "Common/Log.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/Foreach.hpp"

#include "Mesh/CMesh.hpp"
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
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( CMeshPartitioner_test )
{
  CFinfo << "CMeshPartitioner_test" << CFendl;
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
  //meshreader->configure_property("Read Boundaries",false);

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

  //p.configure_property("Number of Partitions", (Uint) 4);
  p.configure_property("Graph Package", std::string("PHG"));
  p.configure_property("Debug Level", 2u);
  BOOST_CHECK(true);
  p.initialize(mesh);
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
  Core::instance().terminate();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

