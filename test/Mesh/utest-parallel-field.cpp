// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for parallel fields"

#include <iomanip>
#include <boost/test/unit_test.hpp>

#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/CEnv.hpp"

#include "Common/Foreach.hpp"
#include "Common/OSystem.hpp"
#include "Common/OSystemLayer.hpp"

#include "Common/MPI/PECommPattern.hpp"
#include "Common/MPI/PEObjectWrapperMultiArray.hpp"
#include "Common/MPI/debug.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshGenerator.hpp"
#include "Mesh/CMeshPartitioner.hpp"
#include "Mesh/CMeshTransformer.hpp"

using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;

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
  mpi::PE::instance().init(m_argc,m_argv);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( parallelize_and_synchronize )
{
  CFinfo << "ParallelFields_test" << CFendl;
  Core::instance().environment().configure_property("log_level",(Uint)DEBUG);

  // Create a mesh

  CMeshGenerator::Ptr meshgenerator = build_component_abstract_type<CMeshGenerator>("CF.Mesh.CSimpleMeshGenerator","1Dgenerator");
  meshgenerator->configure_property("parent",URI("//Root"));
  meshgenerator->configure_property("name",std::string("rect"));
  std::vector<Uint> nb_cells(2);
  std::vector<Real> lengths(2);
  nb_cells[0] = 3;
  nb_cells[1] = 2;
  lengths[0]  = nb_cells[0];
  lengths[1]  = nb_cells[1];
  meshgenerator->configure_property("nb_cells",nb_cells);
  meshgenerator->configure_property("lengths",lengths);
  meshgenerator->configure_property("bdry",false);
  meshgenerator->execute();
  CMesh& mesh = Core::instance().root().get_child("rect").as_type<CMesh>();

//  Or read a mesh
//  CMeshReader::Ptr meshreader = build_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
//  meshreader->configure_property("read_boundaries",false);
//  CMesh::Ptr mesh_ptr = meshreader->create_mesh_from("quadtriag.neu");
//  CMesh& mesh = *mesh_ptr;


  // Create global numbering and connectivity of nodes and elements (necessary for partitioning)
  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumbering","glb_numbering")->transform(mesh);
  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalConnectivity","glb_connectivity")->transform(mesh);

  // Partition the mesh
  CMeshPartitioner::Ptr partitioner_ptr = build_component_abstract_type<CMeshPartitioner>("CF.Mesh.Zoltan.CPartitioner","partitioner");
  CMeshPartitioner& p = *partitioner_ptr;
  p.configure_property("graph_package", std::string("PHG"));
  p.initialize(mesh);
  p.partition_graph();
  p.migrate();


  // Create global node numbering plus ranks (Ranks are necessary for PECommPattern)
  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumberingNodes","glb_node_numbering")->transform(mesh);
  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CGlobalNumberingElements","glb_elem_numbering")->transform(mesh);

  // create a field and assign it to the comm pattern

  CField& field = mesh.create_field("node_rank",CField::Basis::POINT_BASED);

  field.parallelize();

  field.data() = mpi::PE::instance().rank();

  // Synchronize

  // comm_pattern.synchronize(); // via the comm_pattern

  field.synchronize(); // via the field


  BOOST_CHECK(true); // Tadaa

  // Create a field with glb element numbers
  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CreateSpaceP0","create_space_P0")->transform(mesh);
  CField& glb_elem_idx = mesh.create_field("glb_elem_idx",CField::Basis::ELEMENT_BASED,"P0");
  CFieldView& field_view = glb_elem_idx.create_component<CFieldView>("field_view");
  field_view.set_field(glb_elem_idx);
  boost_foreach(const CEntities& elements, glb_elem_idx.field_elements())
  {
    field_view.set_elements(elements);
    for (Uint e=0; e<elements.size(); ++e)
    {
      field_view[e][0] = elements.glb_idx()[e];
    }
  }

  // Create a field with glb node numbers
  CField& glb_node_idx = mesh.create_field("glb_node_idx",CField::Basis::POINT_BASED);
  Uint n=0;
  boost_foreach(const Uint node, glb_node_idx.used_nodes().array())
  {
    glb_node_idx[n++][0] = mesh.nodes().glb_idx()[node];
  }

  // Write the mesh with the fields
  CMeshWriter::Ptr meshwriter = build_component_abstract_type<CMeshWriter>("CF.Mesh.Tecplot.CWriter","meshwriter");

  std::vector<CField::Ptr> fields;
  fields.push_back(field.as_ptr<CField>());
  fields.push_back(glb_elem_idx.as_ptr<CField>());
  fields.push_back(glb_node_idx.as_ptr<CField>());
  meshwriter->set_fields(fields);
  meshwriter->write_from_to(mesh,"parallel_fields.plt");
  CFinfo << "parallel_fields_P*.plt written" << CFendl;

}

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
  mpi::PE::instance().finalize();

  Core::instance().terminate();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

