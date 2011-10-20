// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Signal.hpp"
#include "common/CBuilder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"

#include "common/XML/SignalOptions.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"

#include "RDM/Tags.hpp"

//#include "Mesh/Actions/CInitFieldFunction.hpp"

#include "Physics/PhysModel.hpp"

#include "RDM/RDSolver.hpp"
#include "RDM/Init.hpp"

#include "InitialConditions.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::Mesh;

namespace cf3 {
namespace RDM {


////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < InitialConditions, CAction, LibRDM > InitialConditions_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

InitialConditions::InitialConditions ( const std::string& name ) :
  cf3::Solver::ActionDirector(name)
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

  std::string name = options.value<std::string>("name");

  CAction::Ptr ic = allocate_component< RDM::Init >(name);
  append( ic );


  URI solution_uri = solver().as_type< RDSolver >().fields().get_child( RDM::Tags::solution() ).follow()->uri();

  ic->configure_option( "field", solution_uri );

  std::vector<URI> regions;
  if( options.check("regions") )
  {
    regions = options.array<URI>("regions");
  }
  else // if user did not specify, then use the whole topology (all regions)
  {
    regions.push_back(mesh().topology().uri());
  }
  ic->configure_option("regions" , regions);

  ic->configure_option( RDM::Tags::mesh(), m_mesh.lock()->uri());
  ic->configure_option( RDM::Tags::solver() , m_solver.lock()->uri());
  ic->configure_option( RDM::Tags::physical_model() , m_physical_model.lock()->uri());
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


} // RDM
} // cf3
