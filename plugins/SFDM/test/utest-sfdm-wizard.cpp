// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::SFDM"

#include <boost/test/unit_test.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/CEnv.hpp"
#include "Common/FindComponents.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CSimpleMeshGenerator.hpp"
#include "Mesh/CEntities.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CDomain.hpp"
#include "SFDM/SFDWizard.hpp"
#include "Solver/CModel.hpp"
#include "Solver/CSolver.hpp"

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;
//using namespace CF::Solver::Actions;
using namespace CF::SFDM;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( SFDM_Spaces_Suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Solver )
{
  Core::instance().environment().configure_property("log_level", (Uint)INFO);

  SFDWizard& wizard = Core::instance().root().create_component<SFDWizard>("wizard");
  wizard.create_simulation();

  CModel& model = wizard.model();
  CMesh& mesh = model.domain().create_component<CMesh>("mesh");
  CSimpleMeshGenerator::create_line(mesh, 10., 100);

  Component& iterate = model.solver().access_component("iterate");
  Component& if_milestone = iterate.create_component("7_if_milestone","CF.Solver.Actions.Conditional");
  //if_milestone.create_component("milestone_time_criterion","CF.Solver.Actions.CCriterionMilestoneTime");
  if_milestone.create_component("milestone_time_criterion","CF.Solver.Actions.CCriterionMilestoneIteration");

  CMeshWriter& gmsh_writer = if_milestone.create_component("gmsh_writer","CF.Mesh.Gmsh.CWriter").as_type<CMeshWriter>();
  gmsh_writer.configure_property("mesh",mesh.full_path());
  gmsh_writer.configure_property("file",URI("line.msh"));


  CFdebug << model.tree() << CFendl;

  wizard.prepare_simulation();

  gmsh_writer.set_fields(std::vector<CField::Ptr>(1,mesh.get_child("solution").as_ptr<CField>()));

  std::string gaussian="sigma:="+to_str(1.)+"; mu:="+to_str(5.)+"; exp( -(x-mu)^2/(2*sigma^2) )";

  CFinfo << "\nInitializing solution with [" << gaussian << "]" << CFendl;
  wizard.initialize_solution(std::vector<std::string>(1,gaussian));


  model.configure_option_recursively("milestone_dt",0.5);
  model.configure_option_recursively("milestone_rate",3);

  wizard.start_simulation(3.);

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
