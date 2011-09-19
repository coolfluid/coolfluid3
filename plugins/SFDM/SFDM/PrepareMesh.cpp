// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/Signal.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionArray.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/Actions/CBuildFaces.hpp"

#include "Physics/PhysModel.hpp"

#include "Solver/CSolver.hpp"
#include "SFDM/PrepareMesh.hpp"
#include "SFDM/CreateSFDFields.hpp"
#include "SFDM/SFDSolver.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;
using namespace CF::Mesh::Actions;
using namespace CF::Solver;
using namespace CF::Solver::Actions;

namespace CF {
namespace SFDM {


///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < PrepareMesh, CAction, LibSFDM > PrepareMesh_Builder;

///////////////////////////////////////////////////////////////////////////////////////

PrepareMesh::PrepareMesh ( const std::string& name ) :
  CF::Solver::ActionDirector(name)
{
  mark_basic();

  append( allocate_component<CBuildFaces    >("build_inner_faces") );
  append( allocate_component<CreateSFDFields>("create_sfd_fields") );
}

/////////////////////////////////////////////////////////////////////////////////////

void PrepareMesh::execute()
{
  // configuration of all solver components.
  // This component and its children should be part of it.
  solver().configure_option_recursively(SFDM::Tags::solution_order(),solver().option(SFDM::Tags::solution_order()).value<Uint>());
  solver().configure_option_recursively(SFDM::Tags::mesh(),mesh().uri());
  solver().configure_option_recursively(SFDM::Tags::physical_model(),physical_model().uri());
  solver().configure_option_recursively(SFDM::Tags::solver(),solver().uri());

  // execution of prepare mesh
  ActionDirector::execute();
}

/////////////////////////////////////////////////////////////////////////////////////


} // SFDM
} // CF
