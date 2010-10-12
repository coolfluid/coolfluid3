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

#include "Tools/MeshGeneration/MeshGeneration.hpp"
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
  
  // create a rectangular 2D mesh
  //Tools::MeshGeneration::create_rectangle(*mesh, 10.0, 5.0, 100, 50);
  
  // Read mesh from file
  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("Neu","meshreader");
  boost::filesystem::path fp_in("rotation.neu");
  meshreader->read_from_to(fp_in,mesh);
  
  // create a node-based scalar solution and residual field
  mesh->create_field("solution",1,CField::NODE_BASED);
  mesh->create_field("residual",1,CField::NODE_BASED);
  mesh->create_field("time_step",1,CField::NODE_BASED);
  
  CFinfo << mesh->tree() << CFendl;
  
  // Set the solution field to an initial state
  CSetFieldValues::Ptr set_field = root->create_component_type<CSetFieldValues>("set_field");
  set_field->configure_property("Field",URI("cpath://Root/mesh/solution"));
  //set_field->execute();
  
  // Create a loop over elements with the LDAScheme
  CForAllElementsT<CSchemeLDA>::Ptr elem_loop = root->create_component_type< CForAllElementsT<CSchemeLDA> >("loop_LDA");
  elem_loop->bind();
  
  // Or fully dynamic:
  //   CForAllElements::Ptr elem_loop = root->create_component_type< CForAllElements >("loop_LDA");
  //   elem_loop->create_action("CSchemeLDA");
  
  
  // Configure the loop
  std::vector<URI> regions_to_loop = list_of(URI("cpath://Root/mesh/Base"));
  elem_loop->configure_property("Regions",regions_to_loop);  
  elem_loop->action().configure_property("SolutionField",URI("cpath://Root/mesh/solution"));
  elem_loop->action().configure_property("ResidualField",URI("cpath://Root/mesh/residual"));
  
  // Execute the loop
  elem_loop->execute();
  
  // Write all fields and mesh to file
  CMeshWriter::Ptr meshwriter = create_component_abstract_type<CMeshWriter>("Gmsh","meshwriter");
  boost::filesystem::path fp_out("LDA_scheme.msh");
  meshwriter->write_from_to(mesh,fp_out);
  
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////