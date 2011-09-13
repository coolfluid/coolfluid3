// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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

#include "Common/MPI/CommPattern.hpp"
#include "Common/MPI/CommWrapperMArray.hpp"
#include "Common/MPI/debug.hpp"

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
  Comm::PE::instance().init(m_argc,m_argv);

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
  meshgenerator->configure_option("parent",URI("//Root"));
  meshgenerator->configure_option("name",std::string("rect"));
  std::vector<Uint> nb_cells(2);
  std::vector<Real> lengths(2);
  nb_cells[0] = 10;
  nb_cells[1] = 5;
  lengths[0]  = nb_cells[0];
  lengths[1]  = nb_cells[1];
  meshgenerator->configure_option("nb_cells",nb_cells);
  meshgenerator->configure_option("lengths",lengths);
  meshgenerator->configure_option("bdry",false);
  meshgenerator->execute();
  CMesh& mesh = Core::instance().root().get_child("rect").as_type<CMesh>();
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
      field[n][j] = Comm::PE::instance().rank();

  // Synchronize

  // comm_pattern.synchronize(); // via the comm_pattern

  field.synchronize(); // via the field


  BOOST_CHECK(true); // Tadaa

  // Create a field with glb element numbers
  FieldGroup& elems_P0 = mesh.create_space_and_field_group("elems_P0",FieldGroup::Basis::ELEMENT_BASED,"CF.Mesh.LagrangeP0");
  Field& glb_elem_idx  = elems_P0.create_field("glb_elem");
  Field& elem_rank     = elems_P0.create_field("elem_rank");

  boost_foreach(CElements& elements , elems_P0.elements_range())
  {
    CSpace& space = elems_P0.space(elements);
    for (Uint elem=0; elem<elements.size(); ++elem)
    {
      Uint field_idx = space.indexes_for_element(elem)[0];
      glb_elem_idx[field_idx][0] = 1.;
      elem_rank[field_idx][0] = elements.rank()[elem];
    }
  }

  // Create a field with glb node numbers
  Field& glb_node_idx = mesh.geometry().create_field("glb_node_idx");
  CList<Uint>& glb_idx = mesh.geometry().glb_idx();
  {
    for (Uint n=0; n<glb_node_idx.size(); ++n)
      glb_node_idx[n][0] = glb_idx[n];
  }

  // Write the mesh with the fields

  std::vector<Field::Ptr> fields;
  fields.push_back(field.as_ptr<Field>());
  fields.push_back(glb_elem_idx.as_ptr<Field>());
  fields.push_back(glb_node_idx.as_ptr<Field>());

  CMeshWriter::Ptr tec_writer =
      build_component_abstract_type<CMeshWriter>("CF.Mesh.Tecplot.CWriter","tec_writer");

  tec_writer->set_fields(fields);
  tec_writer->write_from_to(mesh,"parallel_fields.plt");

  CFinfo << "parallel_fields_P*.plt written" << CFendl;

  CMeshWriter::Ptr msh_writer =
      build_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","msh_writer");

  msh_writer->set_fields(fields);
  msh_writer->write_from_to(mesh,"parallel_fields.msh");

  CFinfo << "parallel_fields_P*.msh written" << CFendl;

}

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
  Comm::PE::instance().finalize();

  Core::instance().terminate();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

