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
#include "common/Root.hpp"
#include "common/Libraries.hpp"
#include "common/Environment.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/Field.hpp"
#include "mesh/LoadMesh.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Geometry.hpp"
#include "mesh/Space.hpp"

#include "solver/actions/LibActions.hpp"
#include "solver/actions/CForAllElements.hpp"
#include "solver/actions/CForAllElementsT.hpp"
#include "solver/actions/CForAllNodes2.hpp"
#include "solver/actions/CForAllFaces.hpp"
#include "solver/actions/CLoopOperation.hpp"
#include "solver/actions/CComputeVolume.hpp"
#include "solver/actions/CComputeArea.hpp"

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
  Core::instance().environment().configure_option("log_level",(Uint)DEBUG);
  Root& root = Core::instance().root();
  Mesh::Ptr mesh = root.create_component_ptr<Mesh>("mesh");

  // read mesh from file
  Core::instance().tools().get_child("LoadMesh").as_type<LoadMesh>().load_mesh_into("rotation-tg-p1.neu", *mesh);

  std::vector<URI> regions = list_of(mesh->topology().uri()/URI("rotation/inlet"))
                                    (mesh->topology().uri()/URI("rotation/outlet"));


  // Create a loop over the inlet bc to set the inlet bc to a dirichlet condition
  CLoop::Ptr node_loop2 = root.create_component_ptr< CForAllNodes2 >("node_loop");

  node_loop2->create_loop_operation("cf3.TestActions.CDummyLoopOperation");
  node_loop2->configure_option("regions",regions);

  CFinfo << "\n\n\nNode loop 2 " << CFendl;

  BOOST_CHECK_NO_THROW( node_loop2->execute() );

  BOOST_CHECK(true);

  //root.remove_component(*mesh);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Face_Looping_Test )
{
  Root& root = Core::instance().root();
  Mesh::Ptr mesh = root.create_component_ptr<Mesh>("mesh");

  // read mesh from file
  Core::instance().tools().get_child("LoadMesh").as_type<LoadMesh>().load_mesh_into("rotation-tg-p1.neu", *mesh);

  std::vector<URI> regions = list_of(mesh->topology().uri());

  // Create inner_faces
  MeshTransformer::Ptr facebuilder = build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.BuildFaces","facebuilder");
  //facebuilder->transform(mesh);

  // Create a loop over the inlet bc to set the inlet bc to a dirichlet condition
  CLoop::Ptr face_loop = root.create_component_ptr< CForAllFaces >("face_loop");
  face_loop->create_loop_operation("cf3.TestActions.CDummyLoopOperation");
  face_loop->configure_option("regions",regions);
  CFinfo << "\n\n\nFace loop" << CFendl;

  BOOST_CHECK_NO_THROW( face_loop->execute() );

  MeshTransformer::Ptr info = build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.Info","info");
  info->transform(mesh);

  //root.remove_component(*mesh);

  BOOST_CHECK(true);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( test_CSetFieldValue )
{
  Root& root = Core::instance().root();
  Mesh::Ptr mesh = root.create_component_ptr<Mesh>("mesh2");

  // read mesh from file

  Core::instance().tools().get_child("LoadMesh").as_type<LoadMesh>().load_mesh_into("rotation-tg-p1.neu", *mesh);

  BOOST_CHECK(true);

  Field& field = mesh->geometry().create_field("field");

  CLoop::Ptr node_loop = root.create_component_ptr< CForAllNodes2 >("node_loop");
  node_loop->configure_option("regions",std::vector<URI>(1,mesh->topology().uri()));

/// @todo CSetFieldValues no longer exists, find replacement for node_loop
//  node_loop->create_loop_operation("cf3.solver.actions.CSetFieldValues");
//  node_loop->action("cf3.solver.actions.CSetFieldValues").configure_option("Field",field.uri());
  node_loop->execute();

  BOOST_CHECK(true);

  BOOST_CHECK(find_components_recursively<Cells>(mesh->topology()).size() > 0);

  FieldGroup& cells_P0 = mesh->create_space_and_field_group("cells_P0",FieldGroup::Basis::CELL_BASED,"cf3.mesh.LagrangeP0");
  Field& volumes = cells_P0.create_field("volume");

  BOOST_CHECK(true);


  FieldGroup& faces_P0 = mesh->create_space_and_field_group("faces_P0",FieldGroup::Basis::FACE_BASED, "cf3.mesh.LagrangeP0");
  Field& areas = faces_P0.create_field("area");


  BOOST_CHECK(true);

  CComputeVolume::Ptr compute_volume = root.create_component_ptr<CComputeVolume>("compute_volume");
  BOOST_CHECK(true);
  Elements& elems = root.access_component(mesh->topology().uri()/URI("rotation/fluid/Triag")).as_type<Elements>();
  BOOST_CHECK(true);
  compute_volume->configure_option("volume",volumes.uri());
  BOOST_CHECK(true);
  compute_volume->configure_option("elements",elems.uri());
  BOOST_CHECK(true);
  compute_volume->configure_option("loop_index",12u);
  BOOST_CHECK(true);
  compute_volume->execute();
  BOOST_CHECK(true);

  Space& P0_space = volumes.space(elems);
  BOOST_CHECK_EQUAL( volumes[P0_space.indexes_for_element(12)[0]][0] , 0.0035918050864676932);

  CLoop::Ptr elem_loop = root.create_component_ptr< CForAllElements >("elem_loop");
  elem_loop->configure_option("regions",std::vector<URI>(1,volumes.topology().uri()));

  elem_loop->create_loop_operation("cf3.solver.actions.CComputeVolume");
  elem_loop->action("cf3.solver.actions.CComputeVolume").configure_option("volume",volumes.uri());

  elem_loop->create_loop_operation("cf3.solver.actions.CComputeArea");
  elem_loop->action("cf3.solver.actions.CComputeArea").configure_option("area",areas.uri());

  elem_loop->execute();

  BOOST_CHECK(true);

  std::vector<Field::Ptr> fields;
  fields.push_back(volumes.as_ptr<Field>());
  fields.push_back(field.as_ptr<Field>());
  fields.push_back(areas.as_ptr<Field>());
  MeshWriter::Ptr gmsh_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  gmsh_writer->set_fields(fields);
  gmsh_writer->write_from_to(*mesh,"quadtriag.msh");


  // root.remove_component( *mesh ); // mesh needed for next test
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( test_CForAllElementsT )
{
  Root& root = Core::instance().root();
  Mesh::Ptr mesh = root.get_child_ptr("mesh2")->as_ptr<Mesh>();


  BOOST_CHECK(true);

  Field& field = mesh->get_child("cells_P0").as_type<FieldGroup>().create_field("test_CForAllElementsT","var[1]");

  BOOST_CHECK(true);

  std::vector<URI> topology = list_of(mesh->topology().uri());

  CForAllElementsT<CComputeVolume>::Ptr compute_all_cell_volumes =
    root.create_component_ptr< CForAllElementsT<CComputeVolume> > ("compute_all_cell_volumes");

  BOOST_CHECK(true);

  compute_all_cell_volumes->configure_option("regions",topology);
  BOOST_CHECK(true);

  compute_all_cell_volumes->action().configure_option("volume",field.uri());
  BOOST_CHECK(true);

  compute_all_cell_volumes->execute();

  std::vector<Field::Ptr> fields;
  fields.push_back(field.as_ptr<Field>());
  MeshWriter::Ptr gmsh_writer = build_component_abstract_type<MeshWriter>("cf3.mesh.gmsh.Writer","meshwriter");
  gmsh_writer->set_fields(fields);
  gmsh_writer->write_from_to(*mesh,"test_utest-actions_CForAllElementsT.msh");

  root.remove_component( *mesh );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
