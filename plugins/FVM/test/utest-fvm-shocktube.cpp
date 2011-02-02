// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::FVM"

#include <boost/test/unit_test.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/assign/list_of.hpp>

#include "Common/CreateComponent.hpp"
#include "Common/Log.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "Solver/CDiscretization.hpp"
#include "Solver/CIterativeSolver.hpp"
#include "Solver/CModelUnsteady.hpp"
#include "Solver/CTime.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CDomain.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/Actions/CBuildFaces.hpp"

#include "FVM/ShockTube.hpp"

using namespace boost;
using namespace boost::assign;
using namespace boost::filesystem;
using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;
using namespace CF::FVM;
using namespace CF::Tools::MeshGeneration;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( FVM_Suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( constructor )
{
  // some verbose xml signature
  boost::shared_ptr<XmlDoc> doc = XmlOps::create_doc();
  XmlNode& node  = *XmlOps::goto_doc_node(*doc.get());
  XmlParams p(node);
  p.add_option<std::string>("Model name","shocktube");
  
  ShockTube::Ptr s = allocate_component<ShockTube>("shocktube_wizard");

  CF_DEBUG_POINT;
  
  // 1) create model
  // ---------------
  s->signal_create_model(node);

  CF_DEBUG_POINT;
  
  BOOST_CHECK(true);  
  
  // 2) Load the mesh in Domain
  // --------------------------
  CDomain::Ptr domain = Core::instance().root()->look_component("cpath://Root/shocktube/domain")->as_type<CDomain>();
  CMesh::Ptr mesh = domain->create_component<CMesh>("line");
  create_line(*mesh, 10. , 5u );

  BOOST_CHECK(true);
  
  // 3) Setup model and allocate data
  // --------------------------------
  s->signal_setup_model(node);
  
  BOOST_CHECK(true);
  
  // 4) Configure time
  // -----------------
  CModelUnsteady::Ptr model = Core::instance().root()->get_child<CModelUnsteady>("shocktube");
  model->time().configure_property("Time Step", 2.);
  model->time().configure_property("End Time", 11.);
  //BOOST_CHECK_EQUAL( model->time().dt() , 1.);

  BOOST_CHECK(true);

  // 5) Simulate
  // -----------
  model->simulate();
  
  BOOST_CHECK(true);
  
  // 6) Write mesh
  // -------------
  path file("shocktube.msh");
  
//  meshwriter->configure_property("Fields",fields);
  model->look_component<CMeshWriter>("cpath:./tools/gmsh_writer")->write_from_to(mesh,file);
  
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

