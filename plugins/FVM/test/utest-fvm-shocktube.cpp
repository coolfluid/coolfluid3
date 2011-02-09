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
#include "Mesh/CMeshReader.hpp"
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

  // 1) create model
  // ---------------
  s->signal_create_model(node);
  CModelUnsteady::Ptr model = Core::instance().root()->get_child<CModelUnsteady>("shocktube");
  
  BOOST_CHECK(true);  
  
  // 2) Load the mesh in Domain
  // --------------------------
  // Uint nb_segments = 70;
  // CDomain::Ptr domain = model->get_child<CDomain>("domain");
  // CMesh::Ptr mesh = domain->create_component<CMesh>("line");
  // //create_line(*mesh, 10. , nb_segments );
  // path file_in("line.msh");
  // model->look_component<CMeshReader>("cpath:./tools/gmsh_reader")->read_from_to(file_in,mesh);
  // 
  // model->get_child("IterativeSolver")->properties()["dx"]=10./Real(nb_segments);

  BOOST_CHECK(true);
  
  // 3) Setup model and allocate data
  // --------------------------------
  p.add_option<Uint>("Number of Cells", 100u );
  p.add_option<Real>("End Time", 0.008);
  p.add_option<Real>("Time Step", 0.0004);
  s->signal_setup_model(node);
  
  BOOST_CHECK(true);
  
  // 4) Configure time
  // -----------------
  model->get_child("IterativeSolver")->configure_property("OutputDiagnostics",false);
  
  //BOOST_CHECK_EQUAL( model->time().dt() , 1.);

  BOOST_CHECK(true);

  // 5) Simulate
  // -----------
  model->simulate();
  
  BOOST_CHECK(true);
  
  // 6) Write mesh
  // -------------  
  model->look_component<CMeshWriter>("cpath:./tools/gmsh_writer")->write();
  
  CFinfo << "model:"<<CFendl;
  CFinfo << "------"<<CFendl;
  CFinfo << model->tree() << CFendl;
  CFinfo << "---------------------------------------------------------------------------------" << CFendl;
  CFinfo << "Iterative Solver:" << CFendl;
  CFinfo << "-----------------" << CFendl;
  CFinfo << model->get_child("IterativeSolver")->tree() << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

