// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::mesh::gmsh::Reader parallel"

#include <iostream>
#include <boost/test/unit_test.hpp>

#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "common/BoostAnyConversion.hpp"
#include "common/List.hpp"

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
#include "mesh/MeshAdaptor.hpp"

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
  Core::instance().environment().options().set("log_level",(Uint)INFO);
  Core::instance().environment().options().set("regist_signal_handlers",true);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test )
{
  bool p2=true;
  bool continuous=true;
  Uint dim=2;
  if (1){
  // Generate a mesh
  Handle<Mesh> generated_mesh = Core::instance().root().create_component<Mesh>("mesh");
  boost::shared_ptr< MeshGenerator > generate_mesh = build_component_abstract_type<MeshGenerator>("cf3.mesh.SimpleMeshGenerator","meshgenerator");
  generate_mesh->options().set("nb_cells",std::vector<Uint>(dim,25));
  generate_mesh->options().set("lengths",std::vector<Real>(dim,2.));
  generate_mesh->options().set("mesh",generated_mesh->uri());
  generate_mesh->options().set("bdry",false);
  generate_mesh->execute();

  PE::Comm::instance().barrier();

  if (p2)
  {
  // Create a P2 space and some fields
    if (continuous)
      generated_mesh->create_continuous_space("P2","cf3.mesh.LagrangeP2");
    else
      generated_mesh->create_discontinuous_space("P2","cf3.mesh.LagrangeP2");

    Dictionary& P2 = *generated_mesh->get_child("P2")->handle<Dictionary>();
    CFinfo << "P2 space fully created" << CFendl;

    Field& glb_idx = P2.create_field("glb_node_idx");
    for (Uint n=0; n<P2.size(); ++n)
      glb_idx[n][0] = P2.glb_idx()[n];
  }

  // Write a distributed mesh:
  // -  out-utest-mesh-gmsh-parallel_P0.msh
  // -  out-utest-mesh-gmsh-parallel_P1.msh
  boost::shared_ptr< MeshWriter > write_mesh = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  write_mesh->options().set("mesh",generated_mesh);
  if (p2)
    write_mesh->options().set("fields",std::vector<URI>(1,generated_mesh->uri()/"P2/glb_node_idx"));
  write_mesh->options().set("file",URI("out-utest-mesh-gmsh-parallel.msh"));
  write_mesh->execute();
  }

  PE::Comm::instance().barrier();

  // Read the two mesh-files created previously
  boost::shared_ptr< MeshReader > read_mesh = build_component_abstract_type<MeshReader>("cf3.mesh.gmsh.Reader","meshreader");
  read_mesh->options().set("read_fields",true);

  std::vector< Handle<Mesh> > meshes(PE::Comm::instance().size());
  for (Uint p=0; p<PE::Comm::instance().size(); ++p)
  {
    meshes[p] = Core::instance().root().create_component<Mesh>("mesh"+to_str(p));
    read_mesh->options().set("mesh",meshes[p]);
    read_mesh->options().set("file",URI("out-utest-mesh-gmsh-parallel_P"+to_str(p)+".msh"));
    read_mesh->execute();
    CFinfo << "mesh["<<p<<"]: nb_cells = " << meshes[p]->properties().value_str("global_nb_cells") << CFendl;
    CFinfo << "mesh["<<p<<"]: nb_nodes = " << meshes[p]->properties().value_str("global_nb_nodes") << CFendl;

    PE::Comm::instance().barrier();

    boost::shared_ptr< MeshWriter > write_mesh = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
    write_mesh->options().set("mesh",meshes[p]);

    std::cout << meshes[p]->tree() << std::endl;
    if (p2)
    {
      if (continuous)
        write_mesh->options().set("fields",std::vector<URI>(1,meshes[p]->uri()/"geometry/glb_node_idx"));
      else
        write_mesh->options().set("fields",std::vector<URI>(1,meshes[p]->uri()/"discontinuous_geometry/glb_node_idx"));
    }
    write_mesh->options().set("file",URI("out-P"+to_str(p)+"-utest-mesh-gmsh-parallel.msh"));
    write_mesh->execute();
  }

  // Merge both meshes into one mesh, regions with same name are merged, otherwise added

//  boost::shared_ptr<mesh::actions::MergeMeshes> mesh_merger = allocate_component<mesh::actions::MergeMeshes>("merge_meshes");
  Handle<Mesh> merged_mesh = Core::instance().root().create_component<Mesh>("merged_mesh");

  {
  MeshAdaptor mesh_adaptor(*merged_mesh);

  boost_foreach( const Handle<Mesh>& mesh, meshes)
  {
    mesh_adaptor.combine_mesh(*mesh);
    PE::Comm::instance().barrier();
  }
  cf3_assert(merged_mesh->elements().size());
//  mesh_adaptor.assign_partition_agnostic_global_indices_to_dict(*merged_mesh->get_child("discontinuous_geometry")->handle<Dictionary>());
  mesh_adaptor.remove_duplicate_elements_and_nodes();
  mesh_adaptor.fix_node_ranks();
  mesh_adaptor.finish();
  CFinfo << "merged_mesh: local_nb_cells = " << merged_mesh->properties().value_str("local_nb_cells") << CFendl;
  CFinfo << "merged_mesh: local_nb_nodes = " << merged_mesh->properties().value_str("local_nb_nodes") << CFendl;
  CFinfo << "merged_mesh: global_nb_cells = " << merged_mesh->properties().value_str("global_nb_cells") << CFendl;
  CFinfo << "merged_mesh: global_nb_nodes = " << merged_mesh->properties().value_str("global_nb_nodes") << CFendl;
//  merged_mesh->geometry_fields().update();
  }
  PE::Comm::instance().barrier();
  {
  // Write the merged mesh
  CFinfo << "Write file \"out-merged-utest-mesh-gmsh-parallel.msh\" " << CFendl;
  boost::shared_ptr< MeshWriter > write_mesh = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  write_mesh->options().set("mesh",merged_mesh);
  if (p2)
  {
    if (continuous)
      write_mesh->options().set("fields",std::vector<URI>(1,merged_mesh->uri()/"geometry/glb_node_idx"));
    else
      write_mesh->options().set("fields",std::vector<URI>(1,merged_mesh->uri()/"discontinuous_geometry/glb_node_idx"));
  }
  write_mesh->options().set("file",URI("out-merged-utest-mesh-gmsh-parallel.msh"));
  write_mesh->execute();
  }

  PE::Comm::instance().barrier();

  CFinfo << "Loadbalancing mesh with structure\n" << merged_mesh->tree(true);

  // Loadbalance the merged mesh
  boost::shared_ptr<mesh::actions::LoadBalance> load_balancer = allocate_component<mesh::actions::LoadBalance>("load_balance");
  load_balancer->transform(*merged_mesh);
  CFinfo << "loadbalanced_mesh: nb_cells = " << merged_mesh->properties().value_str("global_nb_cells") << CFendl;
  CFinfo << "loadbalanced_mesh: nb_nodes = " << merged_mesh->properties().value_str("global_nb_nodes") << CFendl;

  PE::Comm::instance().barrier();

  // Write the loadbalanced mesh
  {
    CFinfo << "Write file \"out-loadbalanced-utest-mesh-gmsh-parallel.msh\" " << CFendl;
    boost::shared_ptr< MeshWriter > write_mesh = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
    write_mesh->options().set("mesh",merged_mesh);
    if (continuous)
      write_mesh->options().set("fields",std::vector<URI>(1,merged_mesh->uri()/"geometry/glb_node_idx"));
    else
      write_mesh->options().set("fields",std::vector<URI>(1,merged_mesh->uri()/"discontinuous_geometry/glb_node_idx"));
    write_mesh->options().set("file",URI("out-loadbalanced-utest-mesh-gmsh-parallel.msh"));
    write_mesh->options().set("enable_overlap",false);
    write_mesh->execute();
  }
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

