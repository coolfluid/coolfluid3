// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/Signal.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionArray.hpp"

#include "Mesh/CMesh.hpp"

#include "Physics/PhysModel.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "InitialConditions.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;

namespace CF {
namespace RDM {


///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < InitialConditions, CAction, LibRDM > InitialConditions_Builder;

///////////////////////////////////////////////////////////////////////////////////////

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
  CFinfo << "[RDM] applying initial conditions" << CFendl;

  // apply all registered actions

  CActionDirector::execute();

  // apply all strong BCs

  CAction& strong_bcs =
      access_component( "cpath:../BoundaryConditions/StrongBCs" ).as_type<CAction>();

  strong_bcs.execute();

  // synchronize fields to insure consistency of parallel data

  CAction& synchronize =
      access_component( "cpath:../Actions/Synchronize" ).as_type<CAction>();

  synchronize.execute();
}


void InitialConditions::signal_create_initial_condition ( SignalArgs& node )
{
  SignalOptions options( node );

  std::string name = options.value<std::string>("Name");
  std::string type = options.value<std::string>("Type");

  std::vector<URI> regions = options.array<URI>("Regions");

  /// @todo maybe this should be more specific class not CAction

  CAction::Ptr ic = build_component_abstract_type< CAction >(type,name);

  append( ic );

  ic->configure_option( "regions", regions );

  if( m_mesh.lock() )
    ic->configure_option( Tags::mesh(), m_mesh.lock()->uri());
  if( m_physical_model.lock() )
    ic->configure_option( Tags::physical_model() , m_physical_model.lock()->uri());

}


void InitialConditions::signature_signal_create_initial_condition ( SignalArgs& node )
{
  SignalOptions options( node );

  // name

  options.add_option< OptionT<std::string> >("Name", std::string() )
      ->set_description("Name for created boundary term" );

  // type

  /// @todo should loop on availabe BCs in factory of BCs

  std::vector< boost::any > restricted;

  //  restricted.push_back( std::string("CF.RDM.Core.BcDirichlet") );

  options.add_option< OptionT<std::string> >("Type", std::string("CF.RDM.Core.BcDirichlet") )
      ->set_description("Type for created boundary")
      ->restricted_list() = restricted;

  // regions

  std::vector<URI> dummy;

  /// @todo create here the list of restricted regions, both volume and surface

  options.add_option< OptionArrayT<URI> >("regions", dummy )
      ->set_description("Regions where to apply the boundary condition");
}

///////////////////////////////////////////////////////////////////////////////


} // RDM
} // CF
