// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::SFDM"

#include <boost/test/unit_test.hpp>

#include "Common/CreateComponent.hpp"
#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/FindComponents.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CSimpleMeshGenerator.hpp"
#include "Mesh/CEntities.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CDomain.hpp"
#include "Mesh/Actions/CInitFieldFunction.hpp"
#include "Mesh/Actions/CreateSpaceP0.hpp"
#include "Solver/CModelUnsteady.hpp"
#include "Solver/CSolver.hpp"
#include "Solver/CPhysicalModel.hpp"
#include "SFDM/CreateSpace.hpp"


using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Mesh::Actions;
using namespace CF::Solver;
//using namespace CF::Solver::Actions;
using namespace CF::SFDM;

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE( SFDM_Spaces_Suite )

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( Solver )
{
  CModelUnsteady& model = *Common::Core::instance().root().create_component_ptr<CModelUnsteady>("model");
  CPhysicalModel& physics = model.create_physics("Physics");
  CDomain&        domain  = model.create_domain("Domain");
  CSolver&        solver  = model.create_solver("CF.SFDM.SFDSolver");

  /// Create a mesh consisting of a line with length 1. and 20 divisions
  CMesh& mesh = *domain.create_component_ptr<CMesh>("mesh");
  CSimpleMeshGenerator::create_line(mesh, 1., 20);

  SFDM::CreateSpace::Ptr sfdm_space_creator = allocate_component<SFDM::CreateSpace>("sfdm_space_creator");
  sfdm_space_creator->configure_property("P",2u);
  sfdm_space_creator->transform(mesh);

  solver.configure_property("physical_model",physics.full_path());
  solver.configure_property("Domain",domain.full_path());
  solver.configure_option_recursively("time",model.time().full_path());
  solver.configure_option_recursively("time_accurate",true);

  model.time().configure_property("end_time",0.001);
  model.time().configure_property("time_step",0.0001);

  solver.get_child("iterate").configure_property("verbose",true);
  solver.solve();

  CFinfo << solver.tree() << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

