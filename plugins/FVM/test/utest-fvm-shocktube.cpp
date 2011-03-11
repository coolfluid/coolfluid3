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

#include "Solver/CSolver.hpp"
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
using namespace CF::Common::XML;
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
  SignalFrame frame("", "", "");
  SignalFrame& p = frame.map( Protocol::Tags::key_options() );

  p.set_option<std::string>("model_name","shocktube");

  ShockTube::Ptr s = allocate_component<ShockTube>("shocktube_wizard");

  // 1) create model
  // ---------------
  s->signal_create_model(frame);

  CModelUnsteady::Ptr model = Core::instance().root()->get_child_ptr("shocktube")->as_ptr<CModelUnsteady>();

  BOOST_CHECK(true);

  // 2) Load the mesh in Domain
  // --------------------------
  // Uint nb_segments = 70;
  // CDomain::Ptr domain = model->get_child_ptr("Domain")->as_ptr<CDomain>();
  // CMesh::Ptr mesh = domain->create_component<CMesh>("line");
  // //create_line(*mesh, 10. , nb_segments );
  // path file_in("line.msh");
  // model->access_component_ptr<CMeshReader>("cpath:./tools/gmsh_reader")->read_from_to(file_in,mesh);
  //
  // model->get_child_ptr("IterativeSolver")->properties()["dx"]=10./Real(nb_segments);

  BOOST_CHECK(true);

  // 3) Setup model and allocate data
  // --------------------------------
  p.set_option<Uint>("nb_cells", 100u );
  p.set_option<Real>("end_time", 0.008);
  p.set_option<Real>("time_step", 0.0004);
  s->signal_setup_model(frame);

  BOOST_CHECK(true);

  // 4) Configure time
  // -----------------

  //BOOST_CHECK_EQUAL( model->time().dt() , 1.);

  BOOST_CHECK(true);

  // 5) Simulate
  // -----------
  model->simulate();

  BOOST_CHECK(true);

  // 6) Write mesh
  // -------------
  model->access_component_ptr("cpath:./tools/gmsh_writer")->as_ptr<CMeshWriter>()->write();

  CFinfo << "model:"<<CFendl;
  CFinfo << "------"<<CFendl;
  CFinfo << model->tree() << CFendl;
  CFinfo << "---------------------------------------------------------------------------------" << CFendl;
  CFinfo << "Finite Volume Solver:" << CFendl;
  CFinfo << "---------------------" << CFendl;
  CFinfo << model->get_child_ptr("FiniteVolumeSolver")->tree() << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

