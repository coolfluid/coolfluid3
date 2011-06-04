// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::FVM"

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>

#include "Common/BoostFilesystem.hpp"

#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/CEnv.hpp"
#include "Common/XML/Protocol.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "Solver/CSolver.hpp"
#include "Solver/CModelUnsteady.hpp"
#include "Solver/CTime.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CDomain.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/Actions/CBuildFaces.hpp"
#include "Solver/Actions/CIterate.hpp"

#include "FVM/Core/ShockTube.hpp"

using namespace boost;
using namespace boost::assign;
using namespace boost::filesystem;
using namespace CF;
using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;
using namespace CF::Solver;
using namespace CF::Solver::Actions;
using namespace CF::FVM::Core;
using namespace CF::Tools::MeshGeneration;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( FVM_Suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( constructor )
{

  Core::instance().environment().configure_property("log_level",(Uint)DEBUG);

  // some verbose xml signature
  SignalFrame frame;
  SignalFrame& p = frame.map( Protocol::Tags::key_options() );

  p.set_option<std::string>("model_name","shocktube");

  ShockTube::Ptr s = allocate_component<ShockTube>("shocktube_wizard");

  // 1) create model
  // ---------------
  p.set_option<Uint>("nb_cells", 20u );
  p.set_option<Uint>("dimension", 2u );
  s->signal_create_model(frame);

  BOOST_CHECK(true);

  CModelUnsteady& model = Common::Core::instance().root().get_child("shocktube").as_type<CModelUnsteady>();

  // 4) Configure time
  // -----------------

  model.time().configure_property("end_time",  0.008);
  model.time().configure_property("time_step", 0.008);
  // model.configure_option_recursively("cfl", 1.0);
  find_component_recursively<CIterate>(model).configure_property("max_iter",1u);

  BOOST_CHECK(true);

  // 5) Simulate
  // -----------
  CFinfo << "---------------------------------------------------------------------------------" << CFendl;
  CFinfo << "Finite Volume Solver:" << CFendl;
  CFinfo << "---------------------" << CFendl;
  CFinfo << model.get_child("FiniteVolumeSolver").tree() << CFendl;

  model.simulate();

  BOOST_CHECK(true);

  // 6) Write mesh
  // -------------

  model.access_component("cpath:./tools/mesh_writer").as_type<CMeshWriter>().execute();

  CMeshWriter::Ptr writer = build_component_abstract_type<CMeshWriter>("CF.Mesh.Tecplot.CWriter","tecplot_writer");
  model.get_child("tools").add_component(writer);
  writer->configure_property("fields",std::vector<URI>(1,find_component_recursively_with_tag(model,"solution").uri()));
  writer->configure_property("file",URI(model.name()+".plt"));
  writer->configure_property("mesh",find_component_recursively<CMesh>(model).uri());
  writer->execute();


  // CFinfo << "model:"<<CFendl;
  // CFinfo << "------"<<CFendl;
  // CFinfo << model->tree() << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

