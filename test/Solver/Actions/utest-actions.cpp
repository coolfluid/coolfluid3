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
#include <boost/filesystem/path.hpp>

#include "Common/LibCommon.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/Log.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/CFieldView.hpp"

#include "Solver/Actions/LibActions.hpp"
#include "Solver/Actions/CForAllElements2.hpp"
#include "Solver/Actions/CForAllElementsT2.hpp"
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
  CMesh::Ptr mesh = root->create_component<CMesh>("mesh");
	
  // Read mesh from file
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
  boost::filesystem::path fp_in("rotation-tg-p1.neu");
  meshreader->read_from_to(fp_in,mesh);
  std::vector<URI> regions = list_of(URI("cpath://Root/mesh/topology/default_id1084/inlet"))
                                    (URI("cpath://Root/mesh/topology/default_id1084/outlet"));

  
  // Create a loop over the inlet bc to set the inlet bc to a dirichlet condition
	CLoop::Ptr node_loop2 = root->create_component< CForAllNodes2 >("node_loop");
  node_loop2->create_action("CF.TestActions.CDummyLoopOperation");
	node_loop2->configure_property("Regions",regions);
	CFinfo << "\n\n\nNode loop 2 " << CFendl;
  node_loop2->execute();

	BOOST_CHECK(true);
	
}	

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Face_Looping_Test )
{
  CRoot::Ptr root = Core::instance().root();
  CMesh::Ptr mesh = root->create_component<CMesh>("mesh");
	
  // Read mesh from file
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
  boost::filesystem::path fp_in("rotation-tg-p1.neu");
  meshreader->read_from_to(fp_in,mesh);
  std::vector<URI> regions = list_of(URI("cpath://Root/mesh/topology"));

  // Create inner_faces
  CMeshTransformer::Ptr facebuilder = create_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CBuildFaces","facebuilder");
  //facebuilder->transform(mesh);
  
  // Create a loop over the inlet bc to set the inlet bc to a dirichlet condition
	CLoop::Ptr face_loop = root->create_component< CForAllFaces >("face_loop");
  face_loop->create_action("CF.TestActions.CDummyLoopOperation");
	face_loop->configure_property("Regions",regions);
	CFinfo << "\n\n\nFace loop" << CFendl;
  face_loop->execute();

  BOOST_CHECK(true);

  CMeshTransformer::Ptr info = create_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.CInfo","info");
  info->transform(mesh);
  
  root->remove_component(mesh->name());
	
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Templated_Looping_Test )
{
#if 0
  CRoot::Ptr root = CRoot::create("Root");
  CMesh::Ptr mesh = root->create_component<CMesh>("mesh");

  // Read mesh from file
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
  boost::filesystem::path fp_in("rotation-qd-p1.neu");
  meshreader->read_from_to(fp_in,mesh);

//  root->access_component( "//Root/mesh/Base/rotation-qd" )->rename("rotation");

//  std::cout << root->tree() << std::endl;

//  std::cout << root->list_toc() << std::endl;

  // create a node-based scalar solution and residual field
  mesh->create_field("solution",1,CField::NODE_BASED);
  CField& residual = mesh->create_field("residual",1,CField::NODE_BASED);
  CField& inv_update_coeff = mesh->create_field("update_coeff",1,CField::NODE_BASED);
  
  // Create a loop over the inlet bc to set the inlet bc to a dirichlet condition
	CLoop::Ptr apply_bc = root->create_component< CForAllNodes >("apply_bc");
  apply_bc->create_action("CF.Actions.CSetFieldValues");
	
  std::vector<URI> bc_regions = list_of(URI("cpath://Root/mesh/rotation/inlet"));
	apply_bc->configure_property("Regions",bc_regions);
  apply_bc->action("CF.Actions.CSetFieldValues").configure_property("Field",std::string("solution"));
  
  // Create a loop over elements with the LDAScheme
  CLoop::Ptr elem_loop;
  
  // Static version, templates
//  elem_loop =
//      root->create_component< CForAllElementsT<CSchemeLDA> >("loop_LDA");

  elem_loop =
    root->create_component< CForAllElementsT< CSchemeLDAT< SF::Quad2DLagrangeP1 > > >("loop_LDA");

  // Dynamic version, virtual
  //elem_loop = root->create_component< CForAllElements >("loop_LDA");
  //elem_loop->create_action("CF.Actions.CSchemeLDA");

	BOOST_CHECK(true);

	
  // Configure the elem_loop, and the LDA scheme
  std::vector<URI> regions_to_loop = list_of(URI("cpath://Root/mesh/rotation/fluid"));
  elem_loop->configure_property("Regions",regions_to_loop);  
  elem_loop->action("CF.Actions.CSchemeLDA").configure_property("SolutionField",std::string("solution"));
  elem_loop->action("CF.Actions.CSchemeLDA").configure_property("ResidualField",std::string("residual"));
  elem_loop->action("CF.Actions.CSchemeLDA").configure_property("InverseUpdateCoeff",std::string("update_coeff"));

	BOOST_CHECK(true);

  // Execute the loop
  
  
  CLoop::Ptr take_step = root->create_component<CForAllNodes>("take_step");
  take_step->create_action("CF.Actions.CTakeStep");
	take_step->configure_property("Regions",regions_to_loop);
  take_step->action("CF.Actions.CTakeStep").configure_property("SolutionField",std::string("solution"));
  take_step->action("CF.Actions.CTakeStep").configure_property("ResidualField",std::string("residual"));
  take_step->action("CF.Actions.CTakeStep").configure_property("InverseUpdateCoeff",std::string("update_coeff"));
  
  for ( Uint iter = 0; iter < 500;  ++iter)
  {
    // update coefficient and residual to zero
    // Set the field data of the source field
    BOOST_FOREACH(CTable<Real>& node_data, find_components_recursively_with_tag<CTable<Real> >(residual,"node_data"))
      for (Uint i=0; i<node_data.size(); ++i)
  			node_data[i][0]=0;
    BOOST_FOREACH(CTable<Real>& node_data, find_components_recursively_with_tag<CTable<Real> >(inv_update_coeff,"node_data"))
      for (Uint i=0; i<node_data.size(); ++i)
  			node_data[i][0]=0;

    // apply BC
    apply_bc->execute();  
    // compute element residual distribution
    elem_loop->execute();
    // explicit update
    take_step->execute();
  
    Real rhs_L2=0;
    Uint dof=0;
    BOOST_FOREACH(CTable<Real>& node_data, find_components_recursively_with_tag<CTable<Real> >(residual,"node_data"))
    {    
      for (Uint i=0; i<node_data.size(); ++i)
  		{
        rhs_L2 += node_data[i][0]*node_data[i][0];
        dof++;
  		}
    }
    rhs_L2 = sqrt(rhs_L2)/dof;

    CFinfo << "Iter [" << std::setw(4) << iter << "] L2(rhs) [" << std::setw(12) << rhs_L2 << "]" << CFendl;
  }
	
	
	BOOST_CHECK(true);

  // Write all fields and mesh to file
  CMeshWriter::Ptr meshwriter = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  boost::filesystem::path fp_out("LDA_scheme.msh");
  meshwriter->write_from_to(mesh,fp_out);  

#endif
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( test_CSetFieldValue )
{
  CRoot::Ptr root = Core::instance().root();//CRoot::create("Root");
  CMesh::Ptr mesh = root->create_component<CMesh>("mesh");
	
  // Read mesh from file
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
  boost::filesystem::path fp_in("rotation-tg-p1.neu");
  meshreader->read_from_to(fp_in,mesh);
  
  BOOST_CHECK(true);

  CField2& field = *mesh->create_component<CField2>("field");
  field.configure_property("Topology",mesh->topology().full_path());
  field.configure_property("FieldType",std::string("PointBased"));
  field.create_data_storage();
  
  std::vector<URI> regions = list_of(URI("cpath://Root/mesh/topology"));
  
	CLoop::Ptr node_loop = root->create_component< CForAllNodes2 >("node_loop");
	node_loop->configure_property("Regions",regions);

  node_loop->create_action("CF.Solver.Actions.CSetFieldValues2");
  node_loop->action("CF.Solver.Actions.CSetFieldValues2").configure_property("Field",field.full_path());
  node_loop->execute();
  
  BOOST_CHECK(true);
  
  CField2& volumes = *mesh->create_component<CField2>("volumes");
  volumes.configure_property("Topology",mesh->topology().full_path());
  volumes.configure_property("FieldType",std::string("ElementBased"));
  volumes.create_data_storage();

  CField2& areas = *mesh->create_component<CField2>("areas");
  areas.configure_property("Topology",mesh->topology().full_path());
  areas.configure_property("FieldType",std::string("ElementBased"));
  areas.create_data_storage();

  BOOST_CHECK(true);

  CComputeVolume::Ptr compute_volume = root->create_component<CComputeVolume>("compute_volume");
  CElements& elems = *root->look_component<CElements>(URI("cpath://Root/mesh/topology/default_id1084/fluid/Triag"));
  compute_volume->configure_property("Volume",volumes.full_path());
  BOOST_CHECK(true);
  compute_volume->configure_property("Elements",elems.full_path());
  BOOST_CHECK(true);
  compute_volume->configure_property("LoopIndex",12u);
  BOOST_CHECK(true);
  compute_volume->execute();
  BOOST_CHECK(true);
  CScalarFieldView volume_view("volume_view");
  volume_view.initialize(volumes,elems.as_type<CElements>());
  BOOST_CHECK_EQUAL( volume_view[12] , 0.0035918050864676932);

  CLoop::Ptr elem_loop = root->create_component< CForAllElements2 >("elem_loop");
  elem_loop->configure_property("Regions",regions);
  
  elem_loop->create_action("CF.Solver.Actions.CComputeVolume");
  elem_loop->action("CF.Solver.Actions.CComputeVolume").configure_property("Volume",volumes.full_path());
  
  elem_loop->create_action("CF.Solver.Actions.CComputeArea");
  elem_loop->action("CF.Solver.Actions.CComputeArea").configure_property("Area",areas.full_path());
  
  elem_loop->execute();

  BOOST_CHECK(true);
  
  std::vector<CField2::Ptr> fields;
  fields.push_back(volumes.as_type<CField2>());
  fields.push_back(field.as_type<CField2>());
  fields.push_back(areas.as_type<CField2>());
  boost::filesystem::path fp_out ("quadtriag.msh");
  CMeshWriter::Ptr gmsh_writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  gmsh_writer->set_fields(fields);
  gmsh_writer->write_from_to(mesh,fp_out);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE ( test_CForAllElementsT2 )
{
  CRoot::Ptr root = Core::instance().root();//CRoot::create("Root");
  CMesh::Ptr mesh = root->get_child<CMesh>("mesh");

  BOOST_CHECK(true);

  CField2& field = mesh->create_field2("test_CForAllElementsT2","CellBased","var[1]");

  BOOST_CHECK(true);
  
  std::vector<URI> topology = list_of(URI("cpath://Root/mesh/topology"));
    
  CForAllElementsT2<CComputeVolume>::Ptr compute_all_cell_volumes =
    root->create_component< CForAllElementsT2<CComputeVolume> > ("compute_all_cell_volumes");
  
  BOOST_CHECK(true);
  
  compute_all_cell_volumes->configure_property("Regions",topology);
  BOOST_CHECK(true);
  
  compute_all_cell_volumes->action().configure_property("Volume",field.full_path());
  BOOST_CHECK(true);
  
  compute_all_cell_volumes->execute();
  
  std::vector<CField2::Ptr> fields;
  fields.push_back(field.as_type<CField2>());
  boost::filesystem::path fp_out ("test_utest-actions_CForAllElementsT2.msh");
  CMeshWriter::Ptr gmsh_writer = create_component_abstract_type<CMeshWriter>("CF.Mesh.Gmsh.CWriter","meshwriter");
  gmsh_writer->set_fields(fields);
  gmsh_writer->write_from_to(mesh,fp_out);
  
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
