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

#include "sdm/SDSolver.hpp"
#include "sdm/PrepareMesh.hpp"
#include "sdm/CreateSDFields.hpp"
#include "sdm/Tags.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::mesh;
using namespace cf3::mesh::actions;
using namespace cf3::solver;

namespace cf3 {
namespace sdm {


///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < PrepareMesh, common::Action, LibSDM > PrepareMesh_Builder;

///////////////////////////////////////////////////////////////////////////////////////

PrepareMesh::PrepareMesh ( const std::string& name ) :
  cf3::solver::ActionDirector(name)
{
  mark_basic();

  // Build faces and cell2face and face2cell connectivities
  create_component<BuildFaces>("build_inner_faces")->options().set("store_cell2face",true);

  // renumber elements because of the faces (not strictly necessary)
  // create_component<GlobalNumbering>("renumber");
  
  // Create fields specifically for SD
  create_component<CreateSDFields>("create_sfd_fields");
}

/////////////////////////////////////////////////////////////////////////////////////

void PrepareMesh::execute()
{
  // configuration of all solver components.
  // This component and its children should be part of it.
  solver().configure_option_recursively(sdm::Tags::solution_order(),solver().options().option(sdm::Tags::solution_order()).value<Uint>());
  solver().configure_option_recursively(sdm::Tags::mesh(),mesh().handle<Component>());
  solver().configure_option_recursively(sdm::Tags::regions(),solver().options().option(sdm::Tags::regions()).value< std::vector<URI> >());
  solver().configure_option_recursively(sdm::Tags::physical_model(),physical_model().handle<Component>());
  solver().configure_option_recursively(sdm::Tags::solver(),solver().handle<Component>());

  configure_option_recursively(sdm::Tags::solution_order(),solver().options().option(sdm::Tags::solution_order()).value<Uint>());
  configure_option_recursively(sdm::Tags::mesh(),mesh().handle<Component>());
  configure_option_recursively(sdm::Tags::regions(),solver().options().option(sdm::Tags::regions()).value< std::vector<URI> >());
  configure_option_recursively(sdm::Tags::physical_model(),physical_model().handle<Component>());
  configure_option_recursively(sdm::Tags::solver(),solver().handle<Component>());

  // execution of prepare mesh
  ActionDirector::execute();

  std::vector<URI> fields;
  fields.push_back( follow_link( solver().field_manager().get_child(sdm::Tags::solution()) )->uri() );
  solver().handle<SDSolver>()->time_stepping().post_actions().get_child("Periodic")->configure_option_recursively("fields",fields);
  solver().handle<SDSolver>()->time_stepping().post_actions().get_child("Periodic")->configure_option_recursively("file",URI("sdm_output_${time}.msh"));
}

/////////////////////////////////////////////////////////////////////////////////////


} // sdm
} // cf3
