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
#include "Common/CreateComponent.hpp"
#include "Common/Log.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CFieldView.hpp"

#include "Solver/Actions/LibActions.hpp"
#include "Solver/Actions/CActionDirector.hpp"
#include "Solver/Actions/CForAllElements.hpp"
#include "Solver/Actions/CForAllElementsT.hpp"
#include "Solver/Actions/CForAllNodes2.hpp"
#include "Solver/Actions/CForAllFaces.hpp"
#include "Solver/Actions/CLoopOperation.hpp"
#include "Solver/Actions/CComputeVolume.hpp"
#include "Solver/Actions/CComputeArea.hpp"

#include "Mesh/SF/Triag2DLagrangeP1.hpp"
#include "Mesh/SF/Quad2DLagrangeP1.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;
using namespace CF::Solver::Actions;
using namespace boost::assign;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( TestActionsSuite )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Node_Looping_Test )
{
  CRoot::Ptr root = CRoot::create("Root");
  CMesh::Ptr mesh = root->create_component_ptr<CMesh>("mesh");

  // Read mesh from file
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
  meshreader->read_from_to("rotation-tg-p1.neu",*mesh);
  std::vector<URI> regions = list_of(URI("cpath://Root/mesh/topology/default_id1084/inlet"))
                                    (URI("cpath://Root/mesh/topology/default_id1084/outlet"));


	// Create a loop over the inlet bc to set the inlet bc to a dirichlet condition
	CLoop::Ptr node_loop2 = root->create_component_ptr< CForAllNodes2 >("node_loop");
	node_loop2->create_loop_operation("CF.TestActions.CDummyLoopOperation");
	node_loop2->configure_property("Regions",regions);
	CFinfo << "\n\n\nNode loop 2 " << CFendl;
	node_loop2->execute();

	BOOST_CHECK(true);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Face_Looping_Test )
{
  CRoot& root = Core::instance().root();
  CMesh::Ptr mesh = root.create_component_ptr<CMesh>("mesh");

  // Read mesh from file
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
  meshreader->read_from_to("rotation-tg-p1.neu",*mesh);
  std::vector<URI> regions = list_of(URI("cpath://Root/mesh/topology"));

  // Create inner_faces
  CMeshTransformer::Ptr facebuilder = create_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CBuildFaces","facebuilder");
  //facebuilder->transform(mesh);

	// Create a loop over the inlet bc to set the inlet bc to a dirichlet condition
	CLoop::Ptr face_loop = root.create_component_ptr< CForAllFaces >("face_loop");
	face_loop->create_loop_operation("CF.TestActions.CDummyLoopOperation");
	face_loop->configure_property("Regions",regions);
	CFinfo << "\n\n\nFace loop" << CFendl;
	face_loop->execute();

	BOOST_CHECK(true);

  CMeshTransformer::Ptr info = create_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CInfo","info");
  info->transform(mesh);

  root.remove_component(mesh->name());

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( test_CSetFieldValue )
{
  CRoot& root = Core::instance().root();//CRoot::create("Root");
  CMesh::Ptr mesh = root.create_component_ptr<CMesh>("mesh");

  // Read mesh from file
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
  meshreader->read_from_to("rotation-tg-p1.neu",*mesh);

  BOOST_CHECK(true);

  CField& field = *mesh->create_component_ptr<CField>("field");
  field.configure_property("Topology",mesh->topology().full_path());
  field.configure_property("FieldType",std::string("PointBased"));
  field.create_data_storage();

  std::vector<URI> regions = list_of(URI("cpath://Root/mesh/topology"));

	CLoop::Ptr node_loop = root.create_component_ptr< CForAllNodes2 >("node_loop");
	node_loop->configure_property("Regions",regions);

  node_loop->create_loop_operation("CF.Solver.Actions.CSetFieldValues");
  node_loop->action("CF.Solver.Actions.CSetFieldValues").configure_property("Field",field.full_path());
  node_loop->execute();

  BOOST_CHECK(true);

  CField& volumes = *mesh->create_component_ptr<CField>("volumes");
  volumes.configure_property("Topology",mesh->topology().full_path());
  volumes.configure_property("FieldType",std::string("ElementBased"));
  volumes.create_data_storage();

  CField& areas = *mesh->create_component_ptr<CField>("areas");
  areas.configure_property("Topology",mesh->topology().full_path());
  areas.configure_property("FieldType",std::string("ElementBased"));
  areas.create_data_storage();

  BOOST_CHECK(true);

  CComputeVolume::Ptr compute_volume = root.create_component_ptr<CComputeVolume>("compute_volume");
  BOOST_CHECK(true);
  CElements& elems = root.access_component("//Root/mesh/topology/default_id1084/fluid/Triag").as_type<CElements>();
  BOOST_CHECK(true);
  compute_volume->configure_property("Volume",volumes.full_path());
  BOOST_CHECK(true);
  compute_volume->configure_property("Elements",elems.full_path());
  BOOST_CHECK(true);
  compute_volume->configure_property("LoopIndex",12u);
  BOOST_CHECK(true);
  compute_volume->execute();
  BOOST_CHECK(true);
  CScalarFieldView volume_view("volume_view");
  volume_view.initialize(volumes,elems.as_ptr<CElements>());
  BOOST_CHECK_EQUAL( volume_view[12] , 0.0035918050864676932);

  CLoop::Ptr elem_loop = root.create_component_ptr< CForAllElements >("elem_loop");
  elem_loop->configure_property("Regions",regions);

  elem_loop->create_loop_operation("CF.Solver.Actions.CComputeVolume");
  elem_loop->action("CF.Solver.Actions.CComputeVolume").configure_property("Volume",volumes.full_path());

  elem_loop->create_loop_operation("CF.Solver.Actions.CComputeArea");
  elem_loop->action("CF.Solver.Actions.CComputeArea").configure_property("Area",areas.full_path());

  elem_loop->execute();

  BOOST_CHECK(true);

  std::vector<CField::Ptr> fields;
  fields.push_back(volumes.as_ptr<CField>());
  fields.push_back(field.as_ptr<CField>());
  fields.push_back(areas.as_ptr<CField>());
  CMeshWriter::Ptr gmsh_writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  gmsh_writer->set_fields(fields);
  gmsh_writer->write_from_to(*mesh,"quadtriag.msh");

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( test_CForAllElementsT )
{
  CRoot& root = Core::instance().root();//CRoot::create("Root");
  CMesh::Ptr mesh = root.get_child_ptr("mesh")->as_ptr<CMesh>();

  BOOST_CHECK(true);

  CField& field = mesh->create_field("test_CForAllElementsT",CField::Basis::CELL_BASED,"space[0]","var[1]");

  BOOST_CHECK(true);

  std::vector<URI> topology = list_of(URI("cpath://Root/mesh/topology"));

  CForAllElementsT<CComputeVolume>::Ptr compute_all_cell_volumes =
    root.create_component_ptr< CForAllElementsT<CComputeVolume> > ("compute_all_cell_volumes");

  BOOST_CHECK(true);

  compute_all_cell_volumes->configure_property("Regions",topology);
  BOOST_CHECK(true);

  compute_all_cell_volumes->action().configure_property("Volume",field.full_path());
  BOOST_CHECK(true);

  compute_all_cell_volumes->execute();

  std::vector<CField::Ptr> fields;
  fields.push_back(field.as_ptr<CField>());
  CMeshWriter::Ptr gmsh_writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  gmsh_writer->set_fields(fields);
  gmsh_writer->write_from_to(*mesh,"test_utest-actions_CForAllElementsT.msh");

}

/// Action that sets an integer, for testing purposes
struct SetIntegerAction : CAction
{
  typedef boost::shared_ptr<SetIntegerAction> Ptr;
  typedef boost::shared_ptr<SetIntegerAction const> ConstPtr;
  SetIntegerAction(const std::string& name) : CAction(name), value(0) {}
  static std::string type_name () { return "SetIntegerAction"; }
  virtual void execute()
  {
    value = 1;
  }

  Uint value;
};

BOOST_AUTO_TEST_CASE(TestActionDirector)
{
  CRoot& root = Core::instance().root();
  SetIntegerAction::Ptr test_action = root.create_component_ptr<SetIntegerAction>("testaction");
  CActionDirector::Ptr director = root.create_component_ptr<CActionDirector>("director");
  const std::vector<URI> action_vector(1, test_action->full_path());
  director->configure_property("ActionList", action_vector);
  BOOST_CHECK_EQUAL(test_action->value, 0);
  director->execute();
  BOOST_CHECK_EQUAL(test_action->value, 1);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
