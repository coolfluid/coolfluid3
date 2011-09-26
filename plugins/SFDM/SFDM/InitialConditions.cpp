// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Signal.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionArray.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/Field.hpp"

#include "Physics/PhysModel.hpp"

#include "Solver/CSolver.hpp"

#include "SFDM/Tags.hpp"
#include "SFDM/Init.hpp"

#include "InitialConditions.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;

namespace CF {
namespace SFDM {


////////////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < InitialConditions, CAction, LibSFDM > InitialConditions_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

InitialConditions::InitialConditions ( const std::string& name ) :
  CF::Solver::ActionDirector(name)
{
  mark_basic();

  // signals

  regist_signal( "create_initial_condition" )
      ->connect  ( boost::bind( &InitialConditions::signal_create_initial_condition, this, _1 ) )
      ->signature( boost::bind( &InitialConditions::signature_signal_create_initial_condition, this, _1))
      ->description("creates an initial condition for the solution")
      ->pretty_name("Create Initial Condition");
}



void InitialConditions::execute()
{
  // apply all registered actions

  CActionDirector::execute();

  /// @todo apply all strong BCs

//  CAction& strong_bcs =
//      access_component( "cpath:../BoundaryConditions/StrongBCs" ).as_type<CAction>();

//  strong_bcs.execute();

  // synchronize fields to insure consistency of parallel data

//  CAction& synchronize =
//      access_component( "cpath:../Actions/Synchronize" ).as_type<CAction>();

//  synchronize.execute();
}

Solver::Action& InitialConditions::create_initial_condition(const std::string& name, const std::vector<URI>& regions)
{
  Solver::Action::Ptr ic = allocate_component< SFDM::Init >(name);
  append( ic );

  /// @todo find the field through solver links
  Field& solution = find_component_recursively_with_name<Field>(mesh(),SFDM::Tags::solution());

  ic->configure_option( "solution_field", solution.uri() );

  if( regions.empty() )
  {
    ic->configure_option("regions" , std::vector<URI>(1,mesh().topology().uri()));
  }
  else // if user did not specify, then use the whole topology (all regions)
  {
    ic->configure_option("regions" , regions);
  }

  ic->configure_option( SFDM::Tags::mesh(), m_mesh.lock()->uri());
  ic->configure_option( SFDM::Tags::solver() , m_solver.lock()->uri());
  ic->configure_option( SFDM::Tags::physical_model() , m_physical_model.lock()->uri());

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
    regions.push_back(mesh().topology().uri());
  }

  create_initial_condition(name,regions);
}


void InitialConditions::signature_signal_create_initial_condition ( SignalArgs& node )
{
  SignalOptions options( node );

  // name

  options.add_option< OptionT<std::string> >("name", std::string() )
      ->description("Name for created initial condition" );

  // regions

  std::vector<URI> dummy;

  /// @todo create here the list of restricted regions, both volume and surface

  options.add_option< OptionArrayT<URI> >("regions", dummy )
      ->description("Regions where to apply the initial condition [optional]");
}

////////////////////////////////////////////////////////////////////////////////////////////


} // SFDM
} // CF
