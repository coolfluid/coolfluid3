// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::SFDM"

#include <boost/test/unit_test.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/CRoot.hpp"
#include "common/CEnv.hpp"
#include "common/FindComponents.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CSimpleMeshGenerator.hpp"
#include "Mesh/CEntities.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/WriteMesh.hpp"
#include "Mesh/CDomain.hpp"
#include "SFDM/SFDWizard.hpp"
#include "Solver/CModel.hpp"
#include "Solver/CSolver.hpp"

using namespace cf3;
using namespace cf3::common;
using namespace cf3::Mesh;
using namespace cf3::Solver;
//using namespace cf3::Solver::Actions;
using namespace cf3::SFDM;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( SFDM_Spaces_Suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Solver_1D )
{
  Core::instance().environment().configure_option("log_level", (Uint)INFO);

  SFDWizard& wizard = Core::instance().root().create_component<SFDWizard>("wizard");
  wizard.configure_option("model",std::string("gaussian_1D"));
  wizard.configure_option("solution_state",std::string("CF.AdvectionDiffusion.State1D"));
  wizard.configure_option("roe_state",std::string("CF.AdvectionDiffusion.State1D"));
  wizard.configure_option("dim",1u);
  wizard.create_simulation();

  CModel& model = wizard.model();
  CMesh& mesh = model.domain().create_component<CMesh>("mesh");
  CSimpleMeshGenerator::create_line(mesh, 10., 100);

  Component& iterate = model.solver().access_component("iterate");
  Component& if_milestone = iterate.create_component("7_if_milestone","CF.Solver.Actions.Conditional");
  if_milestone.create_component("milestone_time_criterion","CF.Solver.Actions.CCriterionMilestoneTime");
  //if_milestone.create_component("milestone_time_criterion","CF.Solver.Actions.CCriterionMilestoneIteration");

  WriteMesh& gmsh_writer = if_milestone.create_component("gmsh_writer","CF.Mesh.WriteMesh").as_type<WriteMesh>();
  gmsh_writer.configure_option("mesh",mesh.uri());
  gmsh_writer.configure_option("file",URI("file:line_${date}_iter${iter}_time${time}.msh"));


  CFinfo << model.tree() << CFendl;

  wizard.prepare_simulation();

  gmsh_writer.configure_option("fields",std::vector<URI>(1,mesh.get_child("solution").uri()));

  std::string gaussian="sigma:="+to_str(1.)+"; mu:="+to_str(5.)+"; exp( -(x-mu)^2/(2*sigma^2) )";

  CFinfo << "\nInitializing solution with [" << gaussian << "]" << CFendl;
  wizard.initialize_solution(std::vector<std::string>(1,gaussian));


  model.configure_option_recursively("milestone_dt",0.5);
  model.configure_option_recursively("milestone_rate",3);
  model.configure_option_recursively("stages",1u);
  iterate.configure_option("max_iter",3u);


  wizard.start_simulation(5.);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Solver_2D )
{
  //Core::instance().environment().configure_option("log_level", (Uint)DEBUG);

  SFDWizard& wizard = Core::instance().root().create_component<SFDWizard>("wizard");
  wizard.configure_option("model",std::string("gaussian_2D"));
  wizard.configure_option("solution_state",std::string("CF.AdvectionDiffusion.State2D"));
  wizard.configure_option("roe_state",std::string("CF.AdvectionDiffusion.State2D"));
  wizard.configure_option("dim",2u);
  wizard.configure_option("P",1u);
  wizard.configure_option("RK_stages",3u);
  wizard.configure_option("cfl",1.);
  wizard.create_simulation();

  CModel& model = wizard.model();
  CMesh& mesh = model.domain().create_component<CMesh>("mesh");
  CSimpleMeshGenerator::create_rectangle(mesh, 80., 80., 20, 20);

  Component& iterate = model.solver().access_component("iterate");
  //Component& if_milestone = iterate.create_component("7_if_milestone","CF.Solver.Actions.Conditional");
  //if_milestone.create_component("milestone_time_criterion","CF.Solver.Actions.CCriterionMilestoneTime");
  //if_milestone.create_component("milestone_time_criterion","CF.Solver.Actions.CCriterionMilestoneIteration");

  //WriteMesh& gmsh_writer = if_milestone.create_component("gmsh_writer","CF.Mesh.WriteMesh").as_type<WriteMesh>();
  WriteMesh& gmsh_writer = iterate.create_component("4_gmsh_writer","CF.Mesh.WriteMesh").as_type<WriteMesh>();
  gmsh_writer.configure_option("mesh",mesh.uri());
  gmsh_writer.configure_option("file",URI("file:gaussian_iter${iter}_time${time}.msh"));


  CFinfo << model.tree() << CFendl;

  wizard.prepare_simulation();

  gmsh_writer.configure_option("fields",std::vector<URI>(1,mesh.get_child("solution").uri()));

  std::string gaussian="sigma:="+to_str(6.)+"; mu:="+to_str(40.)+"; exp( -( (x-mu)^2+(y-mu)^2 )/(2*sigma^2) )";

  CFinfo << "\nInitializing solution with [" << gaussian << "]" << CFendl;
  wizard.initialize_solution(std::vector<std::string>(1,gaussian));


  //model.configure_option_recursively("milestone_dt",0.5);
  //model.configure_option_recursively("milestone_rate",3);

  gmsh_writer.execute();

  iterate.configure_option("max_iter",3u);
  wizard.start_simulation(40.);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
