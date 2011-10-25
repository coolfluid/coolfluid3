// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"

#include "common/XML/SignalOptions.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/actions/BuildFaces.hpp"
#include "mesh/actions/GlobalNumbering.hpp"

#include "physics/PhysModel.hpp"

#include "SFDM/SFDSolver.hpp"
#include "SFDM/PrepareMesh.hpp"
#include "SFDM/CreateSFDFields.hpp"
#include "SFDM/Tags.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::mesh;
using namespace cf3::mesh::actions;
using namespace cf3::solver;

namespace cf3 {
namespace SFDM {


///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < PrepareMesh, common::Action, LibSFDM > PrepareMesh_Builder;

///////////////////////////////////////////////////////////////////////////////////////

PrepareMesh::PrepareMesh ( const std::string& name ) :
  cf3::solver::ActionDirector(name)
{
  mark_basic();

  BuildFaces::Ptr build_faces ( allocate_component<BuildFaces>("build_inner_faces") );
  build_faces->configure_option("store_cell2face",true);

  append( build_faces );

  // renumber elements because of the faces (not strictly necessary)
  // append( allocate_component<GlobalNumbering>("glb_numbering") );

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

  std::vector<URI> fields;
  fields.push_back(solver().field_manager().get_child(SFDM::Tags::solution()).follow()->uri());
  solver().as_type<SFDSolver>().time_stepping().post_actions().get_child("Periodic").configure_option_recursively("fields",fields);
  solver().as_type<SFDSolver>().time_stepping().post_actions().get_child("Periodic").configure_option_recursively("file",URI("sfdm_output_${time}.msh"));
}

/////////////////////////////////////////////////////////////////////////////////////


} // SFDM
} // cf3
