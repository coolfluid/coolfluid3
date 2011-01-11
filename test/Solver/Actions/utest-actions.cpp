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
#include "Mesh/CField.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshReader.hpp"

#include "Solver/Actions/LibActions.hpp"
#include "Solver/Actions/CSetFieldValues.hpp"
#include "Solver/Actions/CForAllElementsT.hpp"
#include "Solver/Actions/CForAllElements.hpp"
#include "Solver/Actions/CForAllNodes.hpp"
#include "Solver/Actions/CLoopOperation.hpp"

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

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ConstructorTest )
{
//  CForAllElementsT<CSchemeLDA>::Ptr elem_loop ( allocate_component< CForAllElementsT<CSchemeLDA> >("loop_LDA") );
//  BOOST_CHECK(elem_loop);
//  CLoopOperation::Ptr lda_scheme = create_component_abstract_type<CLoopOperation>("CF.Actions.CSchemeLDA","lda_scheme");
//  BOOST_CHECK(lda_scheme);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Node_Looping_Test )
{
  CRoot::Ptr root = CRoot::create("Root");
  CMesh::Ptr mesh = root->create_component<CMesh>("mesh");
	
  // Read mesh from file
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
  boost::filesystem::path fp_in("rotation-tg.neu");
  meshreader->read_from_to(fp_in,mesh);
  std::vector<URI> regions = list_of(URI("cpath://Root/mesh/default_id1084/inlet"))(URI("cpath://Root/mesh/default_id1084/outlet"));

  
  // Create a loop over the inlet bc to set the inlet bc to a dirichlet condition
	CLoop::Ptr node_loop = root->create_component< CForAllNodes >("node_loop");
  node_loop->create_action("CF.TestActions.CDummyLoopOperation");
	node_loop->configure_property("Regions",regions);
	CFinfo << "\n\n\nNode loop" << CFendl;
  node_loop->execute();

	CLoop::Ptr elem_loop = root->create_component< CForAllElements >("elem_loop");
  elem_loop->create_action("CF.TestActions.CDummyLoopOperation");
	elem_loop->configure_property("Regions",regions);
	CFinfo << "\n\n\nElement loop" << CFendl;
  elem_loop->execute();

	BOOST_CHECK(true);
	
}	

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Templated_Looping_Test )
{
#if 0
  CRoot::Ptr root = CRoot::create("Root");
  CMesh::Ptr mesh = root->create_component<CMesh>("mesh");

  // Read mesh from file
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");
  boost::filesystem::path fp_in("rotation-qd.neu");
  meshreader->read_from_to(fp_in,mesh);

//  root->access_component( "//Root/mesh/Base/rotation-qd" )->rename("rotation");

//  std::cout << root->tree() << std::endl;

//  std::cout << root->list_toc() << std::endl;

  // create a node-based scalar solution and residual field
  mesh->create_field("solution",1,CField::NODE_BASED);
  CField& residual = mesh->create_field("residual",1,CField::NODE_BASED);
  CField& inv_update_coeff = mesh->create_field("inverse_updatecoeff",1,CField::NODE_BASED);
  
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
  elem_loop->action("CF.Actions.CSchemeLDA").configure_property("InverseUpdateCoeff",std::string("inverse_updatecoeff"));

	BOOST_CHECK(true);

  // Execute the loop
  
  
  CLoop::Ptr take_step = root->create_component<CForAllNodes>("take_step");
  take_step->create_action("CF.Actions.CTakeStep");
	take_step->configure_property("Regions",regions_to_loop);
  take_step->action("CF.Actions.CTakeStep").configure_property("SolutionField",std::string("solution"));
  take_step->action("CF.Actions.CTakeStep").configure_property("ResidualField",std::string("residual"));
  take_step->action("CF.Actions.CTakeStep").configure_property("InverseUpdateCoeff",std::string("inverse_updatecoeff"));
  
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

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
