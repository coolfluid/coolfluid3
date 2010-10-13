// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Actions"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/filesystem/path.hpp>

#include "Common/LibCommon.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshReader.hpp"

#include "Actions/LibActions.hpp"
#include "Actions/CSetFieldValues.hpp"
#include "Actions/CForAllElementsT.hpp"
#include "Actions/CForAllElements.hpp"
#include "Actions/CElementOperation.hpp"
#include "Actions/CSchemeLDA.hpp"
#include "Actions/CTakeStep.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Actions;
using namespace boost::assign;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( TestActionsSuite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( ConstructorTest )
{
  CForAllElementsT<CSchemeLDA>::Ptr elem_loop ( new CForAllElementsT<CSchemeLDA> ("loop_LDA") );
  BOOST_CHECK(elem_loop);
  CElementOperation::Ptr lda_scheme = create_component_abstract_type<CElementOperation>("CSchemeLDA","lda_scheme");
  BOOST_CHECK(lda_scheme);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Templated_Looping_Test )
{
  CRoot::Ptr root = CRoot::create("Root");
  CMesh::Ptr mesh = root->create_component_type<CMesh>("mesh");

  // Read mesh from file
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("Neu","meshreader");
  boost::filesystem::path fp_in("rotation.neu");
  meshreader->read_from_to(fp_in,mesh);
  
  // create a node-based scalar solution and residual field
  CField& solution = mesh->create_field("solution",1,CField::NODE_BASED);
  CField& residual = mesh->create_field("residual",1,CField::NODE_BASED);
  CField& inv_update_coeff = mesh->create_field("inverse_updatecoeff",1,CField::NODE_BASED);
  std::vector<std::string> lambda_vars(2);
  // CField& lambda = mesh->create_field("lambda",)
//  CFinfo << mesh->tree() << CFendl;
  
  // Set the solution field to an initial state
  CSetFieldValues::Ptr set_field = root->create_component_type<CSetFieldValues>("set_field");
  set_field->configure_property("Field",URI("cpath://Root/mesh/solution/rotation/inlet"));
  set_field->execute();
  
  // Create a loop over elements with the LDAScheme
  CLoop::Ptr elem_loop;
  
  // Static version, templates
  elem_loop = root->create_component_type< CForAllElementsT<CSchemeLDA> >("loop_LDA");
  
  // Dynamic version, virtual
  //elem_loop = root->create_component_type< CForAllElements >("loop_LDA");
  //elem_loop->create_action("CSchemeLDA");

  // Configure the elem_loop, and the LDA scheme
  std::vector<URI> regions_to_loop = list_of(URI("cpath://Root/mesh/Base/rotation/fluid"));
  elem_loop->configure_property("Regions",regions_to_loop);  
  elem_loop->action("CSchemeLDA").configure_property("SolutionField",URI("cpath://Root/mesh/solution"));
  elem_loop->action("CSchemeLDA").configure_property("ResidualField",URI("cpath://Root/mesh/residual"));
  elem_loop->action("CSchemeLDA").configure_property("InverseUpdateCoeff",URI("cpath://Root/mesh/inverse_updatecoeff"));
  
  // Execute the loop
  
  BOOST_CHECK(true);
  
  CAction::Ptr take_step = root->create_component_type<CTakeStep>("take_step");
  take_step->configure_property("SolutionField",URI("cpath://Root/mesh/solution"));
  take_step->configure_property("ResidualField",URI("cpath://Root/mesh/residual"));
  take_step->configure_property("InverseUpdateCoeff",URI("cpath://Root/mesh/inverse_updatecoeff"));
  
  for ( Uint iter = 0; iter < 1;  ++iter)
  {
    // update coefficient and residual to zero
    // Set the field data of the source field
    BOOST_FOREACH(CArray& node_data, recursive_filtered_range_typed<CArray>(residual,IsComponentTag("node_data")))
    {    
      for (Uint i=0; i<node_data.size(); ++i)
  		{
  			node_data[i][0]=0;
  		}
    }
    BOOST_FOREACH(CArray& node_data, recursive_filtered_range_typed<CArray>(inv_update_coeff,IsComponentTag("node_data")))
    {    
      for (Uint i=0; i<node_data.size(); ++i)
  		{
  			node_data[i][0]=0;
  		}
    }
    // apply BC
    set_field->execute();  
    // compute element residual distribution
    elem_loop->execute();
    // explicit update
    take_step->execute();
  
    Real L2norm=0;
    Uint dof=0;
    BOOST_FOREACH(CArray& node_data, recursive_filtered_range_typed<CArray>(residual,IsComponentTag("node_data")))
    {    
      for (Uint i=0; i<node_data.size(); ++i)
  		{
  			L2norm += node_data[i][0]*node_data[i][0];
        dof++;
  		}
    }
    L2norm = sqrt(L2norm)/dof;
    CFLogVar(L2norm);
  }
  // Write all fields and mesh to file
  CMeshWriter::Ptr meshwriter = create_component_abstract_type<CMeshWriter>("Gmsh","meshwriter");
  boost::filesystem::path fp_out("LDA_scheme.msh");
  meshwriter->write_from_to(mesh,fp_out);
  
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
