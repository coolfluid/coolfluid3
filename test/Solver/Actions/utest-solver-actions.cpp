// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Actions"

#include <iomanip>

#include <boost/test/unit_test.hpp>

#include <boost/assign/list_of.hpp>

#include "Common/LibCommon.hpp"

#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/CLibraries.hpp"
#include "Common/CEnv.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/LoadMesh.hpp"

#include "Solver/Actions/LibActions.hpp"
#include "Solver/Actions/CForAllElements.hpp"
#include "Solver/Actions/CForAllElementsT.hpp"
#include "Solver/Actions/CForAllNodes2.hpp"
#include "Solver/Actions/CForAllFaces.hpp"
#include "Solver/Actions/CLoopOperation.hpp"
#include "Solver/Actions/CComputeVolume.hpp"
#include "Solver/Actions/CComputeArea.hpp"

#include "Mesh/SF/Triag2DLagrangeP1.hpp"
#include "Mesh/SF/Quad2DLagrangeP1.hpp"

using namespace boost::assign;

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;
using namespace CF::Solver::Actions;

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
  CRoot& root = Core::instance().root();
  CMesh::Ptr mesh = root.create_component_ptr<CMesh>("mesh");

  // read mesh from file
  Core::instance().tools().get_child("LoadMesh").as_type<LoadMesh>().load_mesh_into("rotation-tg-p1.neu", *mesh);

  std::vector<URI> regions = list_of(mesh->topology().uri()/URI("rotation/inlet"))
                                    (mesh->topology().uri()/URI("rotation/outlet"));


  // Create a loop over the inlet bc to set the inlet bc to a dirichlet condition
  CLoop::Ptr node_loop2 = root.create_component_ptr< CForAllNodes2 >("node_loop");

  node_loop2->create_loop_operation("CF.TestActions.CDummyLoopOperation");
  node_loop2->configure_option("regions",regions);

  CFinfo << "\n\n\nNode loop 2 " << CFendl;

  BOOST_CHECK_NO_THROW( node_loop2->execute() );

  BOOST_CHECK(true);

  //root.remove_component(*mesh);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Face_Looping_Test )
{
  CRoot& root = Core::instance().root();
  CMesh::Ptr mesh = root.create_component_ptr<CMesh>("mesh");

  // read mesh from file
  Core::instance().tools().get_child("LoadMesh").as_type<LoadMesh>().load_mesh_into("rotation-tg-p1.neu", *mesh);

  std::vector<URI> regions = list_of(mesh->topology().uri());

  // Create inner_faces
  CMeshTransformer::Ptr facebuilder = build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CBuildFaces","facebuilder");
  //facebuilder->transform(mesh);

  // Create a loop over the inlet bc to set the inlet bc to a dirichlet condition
  CLoop::Ptr face_loop = root.create_component_ptr< CForAllFaces >("face_loop");
  face_loop->create_loop_operation("CF.TestActions.CDummyLoopOperation");
  face_loop->configure_option("regions",regions);
  CFinfo << "\n\n\nFace loop" << CFendl;

  BOOST_CHECK_NO_THROW( face_loop->execute() );

  CMeshTransformer::Ptr info = build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CInfo","info");
  info->transform(mesh);

  //root.remove_component(*mesh);

  BOOST_CHECK(true);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( test_CSetFieldValue )
{
  CRoot& root = Core::instance().root();
  CMesh::Ptr mesh = root.create_component_ptr<CMesh>("mesh2");

  // read mesh from file

  Core::instance().tools().get_child("LoadMesh").as_type<LoadMesh>().load_mesh_into("rotation-tg-p1.neu", *mesh);

  BOOST_CHECK(true);

  CField& field = *mesh->create_component_ptr<CField>("field");
  field.configure_option("Topology",mesh->topology().uri());
  field.configure_option("FieldType",std::string("PointBased"));
  field.create_data_storage();

  std::vector<URI> regions = list_of(mesh->topology().uri());

  CLoop::Ptr node_loop = root.create_component_ptr< CForAllNodes2 >("node_loop");
  node_loop->configure_option("regions",regions);

  node_loop->create_loop_operation("CF.Solver.Actions.CSetFieldValues");
  node_loop->action("CF.Solver.Actions.CSetFieldValues").configure_option("Field",field.uri());
  node_loop->execute();

  BOOST_CHECK(true);

  CField& volumes = *mesh->create_component_ptr<CField>("volumes");
  volumes.configure_option("Topology",mesh->topology().uri());
  volumes.configure_option("FieldType",std::string("ElementBased"));
  volumes.create_data_storage();

  CField& areas = *mesh->create_component_ptr<CField>("areas");
  areas.configure_option("Topology",mesh->topology().uri());
  areas.configure_option("FieldType",std::string("ElementBased"));
  areas.create_data_storage();

  BOOST_CHECK(true);

  CComputeVolume::Ptr compute_volume = root.create_component_ptr<CComputeVolume>("compute_volume");
  BOOST_CHECK(true);
  CElements& elems = root.access_component(mesh->topology().uri()/URI("rotation/fluid/Triag")).as_type<CElements>();
  BOOST_CHECK(true);
  compute_volume->configure_option("Volume",volumes.uri());
  BOOST_CHECK(true);
  compute_volume->configure_option("Elements",elems.uri());
  BOOST_CHECK(true);
  compute_volume->configure_option("LoopIndex",12u);
  BOOST_CHECK(true);
  compute_volume->execute();
  BOOST_CHECK(true);
  CScalarFieldView volume_view("volume_view");
  volume_view.initialize(volumes,elems.as_ptr<CElements>());
  BOOST_CHECK_EQUAL( volume_view[12] , 0.0035918050864676932);

  CLoop::Ptr elem_loop = root.create_component_ptr< CForAllElements >("elem_loop");
  elem_loop->configure_option("regions",regions);

  elem_loop->create_loop_operation("CF.Solver.Actions.CComputeVolume");
  elem_loop->action("CF.Solver.Actions.CComputeVolume").configure_option("Volume",volumes.uri());

  elem_loop->create_loop_operation("CF.Solver.Actions.CComputeArea");
  elem_loop->action("CF.Solver.Actions.CComputeArea").configure_option("area",areas.uri());

  elem_loop->execute();

  BOOST_CHECK(true);

  std::vector<CField::Ptr> fields;
  fields.push_back(volumes.as_ptr<CField>());
  fields.push_back(field.as_ptr<CField>());
  fields.push_back(areas.as_ptr<CField>());
  CMeshWriter::Ptr gmsh_writer = build_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  gmsh_writer->set_fields(fields);
  gmsh_writer->write_from_to(*mesh,"quadtriag.msh");


  // root.remove_component( *mesh ); // mesh needed for next test
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( test_CForAllElementsT )
{
  CRoot& root = Core::instance().root();
  CMesh::Ptr mesh = root.get_child_ptr("mesh2")->as_ptr<CMesh>();


  BOOST_CHECK(true);

  CField& field = mesh->create_field("test_CForAllElementsT",CField::Basis::CELL_BASED,"space[0]","var[1]");

  BOOST_CHECK(true);

  std::vector<URI> topology = list_of(mesh->topology().uri());

  CForAllElementsT<CComputeVolume>::Ptr compute_all_cell_volumes =
    root.create_component_ptr< CForAllElementsT<CComputeVolume> > ("compute_all_cell_volumes");

  BOOST_CHECK(true);

  compute_all_cell_volumes->configure_option("regions",topology);
  BOOST_CHECK(true);

  compute_all_cell_volumes->action().configure_option("Volume",field.uri());
  BOOST_CHECK(true);

  compute_all_cell_volumes->execute();

  std::vector<CField::Ptr> fields;
  fields.push_back(field.as_ptr<CField>());
  CMeshWriter::Ptr gmsh_writer = build_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  gmsh_writer->set_fields(fields);
  gmsh_writer->write_from_to(*mesh,"test_utest-actions_CForAllElementsT.msh");

  root.remove_component( *mesh );
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
