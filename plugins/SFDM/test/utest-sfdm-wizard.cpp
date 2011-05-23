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

  CFdebug << model.tree() << CFendl;

  wizard.prepare_simulation();

  std::string gaussian="sigma:="+to_str(1.)+"; mu:="+to_str(5.)+"; exp(-(x-mu)^2/(2*sigma^2)) / exp(-(mu-mu)^2/(2*sigma^2))";

  CFinfo << "\nInitializing solution with [" << gaussian << "]" << CFendl;
  wizard.initialize_solution(std::vector<std::string>(1,gaussian));


  CMeshWriter& gmsh_writer = model.tools().create_component("gmsh_writer","CF.Mesh.Gmsh.CWriter").as_type<CMeshWriter>();
  gmsh_writer.configure_property("mesh",mesh.full_path());
  gmsh_writer.configure_property("file",URI("line.msh"));
  gmsh_writer.set_fields(std::vector<CField::Ptr>(1,mesh.get_child("solution").as_ptr<CField>()));

  wizard.start_simulation(3.);

  gmsh_writer.execute();

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
