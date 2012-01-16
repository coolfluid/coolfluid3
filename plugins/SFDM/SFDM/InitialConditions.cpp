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

#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"

#include "physics/PhysModel.hpp"

#include "solver/CSolver.hpp"

#include "SFDM/Tags.hpp"
#include "SFDM/Init.hpp"

#include "InitialConditions.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::mesh;

namespace cf3 {
namespace SFDM {


////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < InitialConditions, Action, LibSFDM > InitialConditions_Builder;

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

  /// @todo apply all strong BCs

//  Action& strong_bcs =
//      access_component( "cpath:../BoundaryConditions/StrongBCs" ).as_type<Action>();

//  strong_bcs.execute();

  // synchronize fields to insure consistency of parallel data

//  Action& synchronize =
//      access_component( "cpath:../actions/Synchronize" ).as_type<Action>();

//  synchronize.execute();
}

solver::Action& InitialConditions::create_initial_condition(const std::string& name, const std::vector<URI>& regions)
{
  Handle<solver::Action> ic = create_component< SFDM::Init >(name);

  Handle<Field> solution = follow_link(solver().field_manager().get_child(SFDM::Tags::solution()))->handle<Field>();

  ic->options().configure_option( SFDM::Tags::solver() , m_solver);
  ic->options().configure_option( SFDM::Tags::mesh(), m_mesh);
  ic->options().configure_option( "solution_field", solution);

  if( regions.empty() ) // if user did not specify, then use the one from the solver
  {
    ic->options().configure_option("regions" , solver().options().option("regions").value< std::vector<common::URI> >() );
  }
  else
  {
    ic->options().configure_option("regions" , regions);
  }

  ic->options().configure_option( SFDM::Tags::physical_model() , m_physical_model);

  return *ic;
}

void InitialConditions::signal_create_initial_condition ( SignalArgs& node )
{
  SignalOptions options( node );

  std::string name = options.value<std::string>("name");

  std::vector<URI> regions;
  if( options.check("regions") )
  {
    regions = options.array<URI>("regions");
  }
  else // if user did not specify, then use the whole topology (all regions)
  {
    regions = solver().options().option("regions").value< std::vector<common::URI> >();
  }

  create_initial_condition(name,regions);
}


void InitialConditions::signature_signal_create_initial_condition ( SignalArgs& node )
{
  SignalOptions options( node );

  // name

  options.add_option("name", std::string() )
      .description("Name for created initial condition" );

  // regions

  std::vector<URI> dummy;

  /// @todo create here the list of restricted regions, both volume and surface

  options.add_option("regions", dummy )
      .description("Regions where to apply the initial condition [optional]");
}

////////////////////////////////////////////////////////////////////////////////////////////


} // SFDM
} // cf3
