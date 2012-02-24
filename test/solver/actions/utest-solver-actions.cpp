// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::actions"

#include <iomanip>

#include <boost/test/unit_test.hpp>

#include <boost/assign/list_of.hpp>

#include "common/LibCommon.hpp"

#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/Libraries.hpp"
#include "common/Environment.hpp"
#include "common/Group.hpp"
#include "common/OptionList.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/Field.hpp"
#include "mesh/LoadMesh.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/Space.hpp"

#include "solver/actions/LibActions.hpp"
#include "solver/actions/ForAllElements.hpp"
#include "solver/actions/ForAllElementsT.hpp"
#include "solver/actions/ForAllNodes2.hpp"
#include "solver/actions/ForAllFaces.hpp"
#include "solver/actions/LoopOperation.hpp"
#include "solver/actions/ComputeVolume.hpp"
#include "solver/actions/ComputeArea.hpp"

using namespace boost::assign;

using namespace cf3;
using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::solver;
using namespace cf3::solver::actions;

/// @todo create a library for support of the utests
/// @todo move this to a class that all utests global fixtures must inherit from
struct CoreInit {

  /// global initiate
  CoreInit()
  {
    using namespace boost::unit_test::framework;
    Core::instance().initiate( master_test_suite().argc, master_test_suite().argv);
  }

  /// global tear-down
  ~CoreInit()
  {
    Core::instance().terminate();
  }

};

//////////////////////////////////////////////////////////////////////////////

BOOST_GLOBAL_FIXTURE( CoreInit )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( TestActionsSuite )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Node_Looping_Test )
{
  Core::instance().environment().options().configure_option("log_level",(Uint)DEBUG);
  Component& root = Core::instance().root();
  Handle<Mesh> mesh = root.create_component<Mesh>("mesh");

  // read mesh from file
  Core::instance().tools().get_child("LoadMesh")->handle<LoadMesh>()->load_mesh_into("rotation-tg-p1.neu", *mesh);

  std::vector<URI> regions = list_of(mesh->topology().uri()/URI("rotation/inlet"))
                                    (mesh->topology().uri()/URI("rotation/outlet"));


  // Create a loop over the inlet bc to set the inlet bc to a dirichlet condition
  Handle<Loop> node_loop2 = root.create_component< ForAllNodes2 >("node_loop");

  node_loop2->create_loop_operation("cf3.TestActions.DummyLoopOperation");
  node_loop2->options().configure_option("regions",regions);

  CFinfo << "\n\n\nNode loop 2 " << CFendl;

  BOOST_CHECK_NO_THROW( node_loop2->execute() );

  BOOST_CHECK(true);

  //root.remove_component(*mesh);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Face_Looping_Test )
{
  Component& root = Core::instance().root();
  Handle<Mesh> mesh = root.create_component<Mesh>("mesh");

  // read mesh from file
  Core::instance().tools().get_child("LoadMesh")->handle<LoadMesh>()->load_mesh_into("rotation-tg-p1.neu", *mesh);

  std::vector<URI> regions = list_of(mesh->topology().uri());

  // Create inner_faces
  boost::shared_ptr< MeshTransformer > facebuilder = build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.BuildFaces","facebuilder");
  //facebuilder->transform(mesh);

  // Create a loop over the inlet bc to set the inlet bc to a dirichlet condition
  Handle<Loop> face_loop = root.create_component< ForAllFaces >("face_loop");
  face_loop->create_loop_operation("cf3.TestActions.DummyLoopOperation");
  face_loop->options().configure_option("regions",regions);
  CFinfo << "\n\n\nFace loop" << CFendl;

  BOOST_CHECK_NO_THROW( face_loop->execute() );

  boost::shared_ptr< MeshTransformer > info = build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.Info","info");
  info->transform(mesh);

  //root.remove_component(*mesh);

  BOOST_CHECK(true);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( test_CSetFieldValue )
{
  Component& root = Core::instance().root();
  Handle<Mesh> mesh = root.create_component<Mesh>("mesh2");

  // read mesh from file

  Core::instance().tools().get_child("LoadMesh")->handle<LoadMesh>()->load_mesh_into("rotation-tg-p1.neu", *mesh);

  BOOST_CHECK(true);

  Field& field = mesh->geometry_fields().create_field("field");

  Handle<Loop> node_loop = root.create_component< ForAllNodes2 >("node_loop");
  node_loop->options().configure_option("regions",std::vector<URI>(1,mesh->topology().uri()));

/// @todo CSetFieldValues no longer exists, find replacement for node_loop
//  node_loop->create_loop_operation("cf3.solver.actions.CSetFieldValues");
//  node_loop->action("cf3.solver.actions.CSetFieldValues").options().configure_option("Field",field.uri());
  node_loop->execute();

  BOOST_CHECK(true);

  BOOST_CHECK(find_components_recursively<Cells>(mesh->topology()).size() > 0);

  Dictionary& cells_P0 = mesh->create_discontinuous_space("cells_P0","cf3.mesh.LagrangeP0");
  Field& volumes = cells_P0.create_field("volume");

  BOOST_CHECK(true);


  Dictionary& faces_P0 = mesh->create_discontinuous_space("faces_P0", "cf3.mesh.LagrangeP0");
  Field& areas = faces_P0.create_field("area");


  BOOST_CHECK(true);

  Handle<ComputeVolume> compute_volume = root.create_component<ComputeVolume>("compute_volume");
  BOOST_CHECK(true);
  Elements& elems = *root.access_component(mesh->topology().uri()/URI("rotation/fluid/Triag"))->handle<Elements>();
  BOOST_CHECK(true);
  compute_volume->options().configure_option("volume",volumes.uri());
  BOOST_CHECK(true);
  compute_volume->options().configure_option("elements",elems.uri());
  BOOST_CHECK(true);
  compute_volume->options().configure_option("loop_index",12u);
  BOOST_CHECK(true);
  compute_volume->execute();
  BOOST_CHECK(true);

  const Space& P0_space = volumes.space(elems);
  BOOST_CHECK_EQUAL( volumes[P0_space.connectivity()[12][0]][0] , 0.0035918050864676932);

  Handle<Loop> elem_loop = root.create_component< ForAllElements >("elem_loop");
  elem_loop->options().configure_option("regions",std::vector<URI>(1,mesh->topology().uri()));

  elem_loop->create_loop_operation("cf3.solver.actions.ComputeVolume");
  elem_loop->action("cf3.solver.actions.ComputeVolume").options().configure_option("volume",volumes.uri());

  elem_loop->create_loop_operation("cf3.solver.actions.ComputeArea");
  elem_loop->action("cf3.solver.actions.ComputeArea").options().configure_option("area",areas.uri());

  elem_loop->execute();

  BOOST_CHECK(true);

  std::vector<URI> fields;
  fields.push_back(volumes.uri());
  fields.push_back(field.uri());
  fields.push_back(areas.uri());
  boost::shared_ptr< MeshWriter > gmsh_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  gmsh_writer->options().configure_option("fields",fields);
  gmsh_writer->options().configure_option("mesh",mesh);
  gmsh_writer->options().configure_option("file",URI("quadtriag.msh"));
  gmsh_writer->execute();

  // root.remove_component( *mesh ); // mesh needed for next test
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( test_ForAllElementsT )
{
  Component& root = Core::instance().root();
  Handle< Mesh > mesh = root.get_child("mesh2")->handle<Mesh>();


  BOOST_CHECK(true);

  Field& field = mesh->get_child("cells_P0")->handle<Dictionary>()->create_field("test_ForAllElementsT","var[1]");

  BOOST_CHECK(true);

  std::vector<URI> topology = list_of(mesh->topology().uri());

  Handle< ForAllElementsT<ComputeVolume> > compute_all_cell_volumes =
    root.create_component< ForAllElementsT<ComputeVolume> > ("compute_all_cell_volumes");

  BOOST_CHECK(true);

  compute_all_cell_volumes->options().configure_option("regions",topology);
  BOOST_CHECK(true);

  compute_all_cell_volumes->action().options().configure_option("volume",field.uri());
  BOOST_CHECK(true);

  compute_all_cell_volumes->execute();

  std::vector<URI> fields;
  fields.push_back(field.uri());
  boost::shared_ptr< MeshWriter > gmsh_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  gmsh_writer->options().configure_option("fields",fields);
  gmsh_writer->options().configure_option("mesh",mesh);
  gmsh_writer->options().configure_option("file",URI("test_utest-actions_ForAllElementsT.msh"));
  gmsh_writer->execute();

  root.remove_component( *mesh );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
