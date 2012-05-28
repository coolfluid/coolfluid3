// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"

#include "common/XML/SignalOptions.hpp"

#include "mesh/Field.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"

#include "RDM/Tags.hpp"

//#include "mesh/actions/InitFieldFunction.hpp"

#include "physics/PhysModel.hpp"

#include "RDM/RDSolver.hpp"
#include "RDM/Init.hpp"

#include "InitialConditions.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::mesh;

namespace cf3 {
namespace RDM {


////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < InitialConditions, common::Action, LibRDM > InitialConditions_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

InitialConditions::InitialConditions ( const std::string& name ) :
  cf3::solver::ActionDirector(name)
{
  mark_basic();

  // signals

  regist_signal( "create_initial_condition" )
      .connect  ( boost::bind( &InitialConditions::signal_create_initial_condition, this, _1 ) )
      .signature( boost::bind( &InitialConditions::signature_signal_create_initial_condition, this, _1))
      .description("creates an initial condition for the solution")
      .pretty_name("Create Initial Condition");
}



void InitialConditions::execute()
{
  // apply all registered actions

  ActionDirector::execute();

  // apply all strong BCs

  Handle<Action> strong_bcs(access_component( "cpath:../BoundaryConditions/StrongBCs" ));

  strong_bcs->execute();

  // synchronize fields to insure consistency of parallel data

  Handle<Action> synchronize(access_component( "cpath:../actions/Synchronize" ));

  synchronize->execute();
}


void InitialConditions::signal_create_initial_condition ( SignalArgs& node )
{
  SignalOptions options( node );

  std::string name = options.value<std::string>("name");

  Handle<Action> ic = create_component< RDM::Init >(name);


  Handle<Field> solution_field( follow_link( solver().handle<RDSolver>()->fields().get_child( RDM::Tags::solution() ) ) );

  ic->options().set( "field", solution_field );

  std::vector<URI> regions;
  if( options.check("regions") )
  {
    regions = options.array<URI>("regions");
  }
  else // if user did not specify, then use the whole topology (all regions)
  {
    regions.push_back(mesh().topology().uri());
  }
  ic->options().set("regions" , regions);

  ic->options().set( RDM::Tags::mesh(), m_mesh );
  ic->options().set( RDM::Tags::solver() , m_solver );
  ic->options().set( RDM::Tags::physical_model() , m_physical_model );
}


void InitialConditions::signature_signal_create_initial_condition ( SignalArgs& node )
{
  SignalOptions options( node );

  // name

  options.add("name", std::string() )
      .description("Name for created initial condition" );

  // regions

  std::vector<URI> dummy;

  /// @todo create here the list of restricted regions, both volume and surface

  options.add("regions", dummy )
      .description("Regions where to apply the initial condition [optional]");
}

////////////////////////////////////////////////////////////////////////////////////////////


} // RDM
} // cf3
