// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionArray.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "RDM/Core/BoundaryTerm.hpp"

#include "BoundaryConditions.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;

namespace CF {
namespace RDM {
namespace Core {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < BoundaryConditions, CAction, LibCore > BoundaryConditions_Builder;

///////////////////////////////////////////////////////////////////////////////////////

BoundaryConditions::BoundaryConditions ( const std::string& name ) :
  Common::CActionDirector(name)
{
  mark_basic();

  // subcomponents

  m_weak_bcs   = create_static_component_ptr<CActionDirector>("WeakBCs");
  m_strong_bcs = create_static_component_ptr<CActionDirector>("StrongBCs");

  // signals

  regist_signal( "create_boundary_condition" )
      ->connect  ( boost::bind( &BoundaryConditions::signal_create_boundary_condition, this, _1 ) )
      ->signature( boost::bind( &BoundaryConditions::signature_signal_create_boundary_condition, this, _1))
      ->description("creates a boundary condition for the solution")
      ->pretty_name("Create Boundary Condition");
}


void BoundaryConditions::execute()
{
  CFinfo << "[RDM] applying boundary conditions " << CFendl;

  // apply first weak bcs, since they do not set any value directly

  m_weak_bcs->execute();

  // strong bcs need to be updated last

  m_strong_bcs->execute();
}


void BoundaryConditions::signal_create_boundary_condition ( SignalArgs& node )
{
  SignalOptions options( node );

  std::string name = options.value<std::string>("Name");
  std::string type = options.value<std::string>("Type");

  std::vector<URI> regions = options.array<URI>("Regions");

  RDM::BoundaryTerm::Ptr bterm = build_component_abstract_type<RDM::BoundaryTerm>(type,name);

  add_component( bterm ); // stays owned here

  // place link either in the weak or strong bcs

  if ( bterm->is_weak() )
    m_weak_bcs->append( bterm );
  else
    m_strong_bcs->append( bterm );

  bterm->configure_option("regions" , regions);

//  if( m_mesh.lock() )
//    bterm->configure_option("mesh", m_mesh.lock()->uri());
//  if( m_physical_model.lock() )
//    bterm->configure_option("physical_model" , m_physical_model.lock()->uri());

}


void BoundaryConditions::signature_signal_create_boundary_condition ( SignalArgs& node )
{
  SignalOptions options( node );

  // name

  options.add_option< OptionT<std::string> >("Name", std::string() )
      ->set_description("Name for created boundary term" );

  /// @todo should loop on availabe BCs in factory of BCs

  // type

  std::vector< boost::any > restricted;
//  restricted.push_back( std::string("CF.RDM.Core.BcDirichlet") );
  options.add_option< OptionT<std::string> >("Type", std::string("CF.RDM.Core.BcDirichlet") )
      ->set_description("Type for created boundary")
      ->restricted_list() = restricted;

  // regions
  std::vector<URI> dummy;
  /// @todo create here the list of restricted surface regions
  options.add_option< OptionArrayT<URI> >("regions", dummy )
      ->set_description("Regions where to apply the boundary condition");
}

////////////////////////////////////////////////////////////////////////////////

} // Core
} // RDM
} // CF
