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


#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/MeshGenerator.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/Field.hpp"
#include "mesh/Entities.hpp"
#include "mesh/Space.hpp"
#include "mesh/Dictionary.hpp"

#include "mesh/actions/MergeMeshes.hpp"
#include "mesh/actions/LoadBalance.hpp"

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
  Core::instance().environment().options().configure_option("log_level",(Uint)INFO);
  Core::instance().environment().options().configure_option("regist_signal_handlers",true);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test )
{
  Uint dim=2;

  // Generate a mesh
  Handle<Mesh> mesh = Core::instance().root().create_component<Mesh>("mesh");
  boost::shared_ptr< MeshGenerator > generate_mesh = build_component_abstract_type<MeshGenerator>("cf3.mesh.SimpleMeshGenerator","meshgenerator");
  generate_mesh->options().configure_option("nb_cells",std::vector<Uint>(dim,100));
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

  std::vector< Handle<Mesh> > meshes(PE::Comm::instance().size());
  for (Uint p=0; p<PE::Comm::instance().size(); ++p)
  {
    meshes[p] = Core::instance().root().create_component<Mesh>("mesh"+to_str(p));
    read_mesh->options().configure_option("mesh",meshes[p]);
    read_mesh->options().configure_option("file",URI("out-utest-mesh-gmsh-parallel_P"+to_str(p)+".msh"));
    read_mesh->execute();
    CFinfo << "mesh["<<p<<"]: nb_cells = " << meshes[p]->properties().value_str("nb_cells") << CFendl;
    CFinfo << "mesh["<<p<<"]: nb_nodes = " << meshes[p]->properties().value_str("nb_nodes") << CFendl;
  }

  // Merge both meshes into one mesh, regions with same name are merged, otherwise added
  boost::shared_ptr<mesh::actions::MergeMeshes> mesh_merger = allocate_component<mesh::actions::MergeMeshes>("merge_meshes");
  Handle<Mesh> merged_mesh = Core::instance().root().create_component<Mesh>("merged_mesh");
  boost_foreach( const Handle<Mesh>& mesh, meshes)
  {
    mesh_merger->merge_mesh(*mesh,*merged_mesh);
  }
  mesh_merger->fix_ranks(*merged_mesh);
  CFinfo << "merged_mesh: nb_cells = " << merged_mesh->properties().value_str("nb_cells") << CFendl;
  CFinfo << "merged_mesh: nb_nodes = " << merged_mesh->properties().value_str("nb_nodes") << CFendl;

  // Write the merged mesh
  write_mesh->options().configure_option("mesh",merged_mesh);
  write_mesh->options().configure_option("file",URI("out-merged-utest-mesh-gmsh-parallel.msh"));
  write_mesh->execute();

  // Loadbalance the merged mesh
  boost::shared_ptr<mesh::actions::LoadBalance> load_balancer = allocate_component<mesh::actions::LoadBalance>("load_balance");
  load_balancer->transform(*merged_mesh);

  // Write the loadbalanced mesh
  write_mesh->options().configure_option("file",URI("out-loadbalanced-utest-mesh-gmsh-parallel.msh"));
  write_mesh->execute();
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

