// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::mesh::gmsh::Reader parallel"

#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "common/BoostAnyConversion.hpp"

#include "math/VariablesDescriptor.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/MeshGenerator.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/Field.hpp"
#include "mesh/Entities.hpp"
#include "mesh/Space.hpp"
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

struct gmshReaderMPITests_Fixture
{
  /// common setup for each test case
  gmshReaderMPITests_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~gmshReaderMPITests_Fixture()
  {
  }
  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here
  int    m_argc;
  char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( gmshReaderMPITests_TestSuite, gmshReaderMPITests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
  Core::instance().initiate(m_argc,m_argv);
  PE::Comm::instance().init(m_argc,m_argv);
  Core::instance().environment().options().configure_option("log_level",4u);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test )
{
  Uint dim=2;

  // Generate a mesh
  Handle<Mesh> mesh = Core::instance().root().create_component<Mesh>("mesh");
  boost::shared_ptr< MeshGenerator > generate_mesh = build_component_abstract_type<MeshGenerator>("cf3.mesh.SimpleMeshGenerator","meshgenerator");
  generate_mesh->options().configure_option("nb_cells",std::vector<Uint>(dim,2));
  generate_mesh->options().configure_option("lengths",std::vector<Real>(dim,2.));
  generate_mesh->options().configure_option("mesh",mesh->uri());
  generate_mesh->execute();

  // Write a distributed mesh:
  // -  out-utest-mesh-gmsh-parallel_P0.msh
  // -  out-utest-mesh-gmsh-parallel_P1.msh
  boost::shared_ptr< MeshWriter > write_mesh = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  write_mesh->options().configure_option("mesh",mesh);
  write_mesh->options().configure_option("file",URI("out-utest-mesh-gmsh-parallel.msh"));
  write_mesh->execute();


  // Read the two mesh-files created previously
  boost::shared_ptr< MeshReader > read_mesh = build_component_abstract_type<MeshReader>("cf3.mesh.gmsh.Reader","meshreader");
  Handle<Mesh> mesh0 = Core::instance().root().create_component<Mesh>("mesh0");
  read_mesh->options().configure_option("mesh",mesh0);
  read_mesh->options().configure_option("file",URI("out-utest-mesh-gmsh-parallel_P0.msh"));
  read_mesh->execute();
  CFinfo << "mesh0: nb_cells = " << mesh0->properties().value_str("nb_cells") << CFendl;
  CFinfo << "mesh0: nb_nodes = " << mesh0->properties().value_str("nb_nodes") << CFendl;
  Handle<Mesh> mesh1 = Core::instance().root().create_component<Mesh>("mesh1");
  read_mesh->options().configure_option("mesh",mesh1);
  read_mesh->options().configure_option("file",URI("out-utest-mesh-gmsh-parallel_P1.msh"));
  read_mesh->execute();
  CFinfo << "mesh1: nb_cells = " << mesh1->properties().value_str("nb_cells") << CFendl;
  CFinfo << "mesh1: nb_nodes = " << mesh1->properties().value_str("nb_nodes") << CFendl;

  // Merge both meshes into one mesh, regions with same name are merged, otherwise added
  Handle<Mesh> merged_mesh = Core::instance().root().create_component<Mesh>("merged_mesh");



  // Remove duplicate nodes (expensive step)



  // Write the merged mesh

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

