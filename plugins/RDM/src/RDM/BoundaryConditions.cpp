// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/CBuilder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/Signal.hpp"

#include "common/XML/SignalOptions.hpp"

#include "mesh/CMesh.hpp"

#include "Physics/PhysModel.hpp"

#include "Solver/CSolver.hpp"
#include "RDM/Tags.hpp"

#include "RDM/BoundaryTerm.hpp"

#include "BoundaryConditions.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::mesh;

namespace cf3 {
namespace RDM {


////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < BoundaryConditions, Action, LibRDM > BoundaryConditions_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

BoundaryConditions::BoundaryConditions ( const std::string& name ) :
  cf3::Solver::ActionDirector(name)
{
  mark_basic();

  // subcomponents

  m_weak_bcs   = create_static_component_ptr<ActionDirector>("WeakBCs");
  m_strong_bcs = create_static_component_ptr<ActionDirector>("StrongBCs");

  // signals

  regist_signal( "create_boundary_condition" )
      ->connect  ( boost::bind( &BoundaryConditions::signal_create_boundary_condition, this, _1 ) )
      ->signature( boost::bind( &BoundaryConditions::signature_signal_create_boundary_condition, this, _1))
      ->description("creates a boundary condition for the solution")
      ->pretty_name("Create Boundary Condition");
}


void BoundaryConditions::execute()
{
//  CFinfo << "[RDM] applying boundary conditions " << CFendl;

  // apply first weak bcs, since they do not set any value directly

//  CFinfo << "[RDM] weak bcs " << CFendl;

  m_weak_bcs->execute();

  // strong bcs need to be updated last

//  CFinfo << "[RDM] strong bcs " << CFendl;

  m_strong_bcs->execute();
}

RDM::BoundaryTerm& BoundaryConditions::create_boundary_condition( const std::string& type,
                                                                  const std::string& name,
                                                                  const std::vector<URI>& regions )
{
  RDM::BoundaryTerm::Ptr bterm = build_component_abstract_type<RDM::BoundaryTerm>(type,name);

  add_component( bterm ); // stays owned here

  // place link either in the weak or strong bcs

  if ( bterm->is_weak() )
    m_weak_bcs->append( bterm );
  else
    m_strong_bcs->append( bterm );

  bterm->configure_option("regions" , regions);

  bterm->configure_option( RDM::Tags::mesh(), m_mesh.lock()->uri());
  bterm->configure_option( RDM::Tags::solver() , m_solver.lock()->uri());
  bterm->configure_option( RDM::Tags::physical_model() , m_physical_model.lock()->uri());

  return *bterm;
}

void BoundaryConditions::signal_create_boundary_condition ( SignalArgs& node )
{
  SignalOptions options( node );

  std::string name = options.value<std::string>("name");
  std::string type = options.value<std::string>("type");

  std::vector<URI> regions = options.array<URI>("regions");

  create_boundary_condition( type, name, regions );
}


void BoundaryConditions::signature_signal_create_boundary_condition ( SignalArgs& node )
{
  SignalOptions options( node );

  // name

  options.add_option< OptionT<std::string> >("name", std::string() )
      ->description("Name for created boundary term" );

  /// @todo should loop on availabe BCs in factory of BCs

  // type

  std::vector< boost::any > restricted;
//  restricted.push_back( std::string("CF.RDM.BcDirichlet") );
  options.add_option< OptionT<std::string> >("type", std::string("CF.RDM.BcDirichlet") )
      ->description("Type for created boundary")
      ->restricted_list() = restricted;

  // regions
  std::vector<URI> dummy;
  /// @todo create here the list of restricted surface regions
  options.add_option< OptionArrayT<URI> >("regions", dummy )
      ->description("Regions where to apply the boundary condition");
}

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3
