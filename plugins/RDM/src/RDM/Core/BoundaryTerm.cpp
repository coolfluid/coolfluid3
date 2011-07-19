// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Signal.hpp"

#include "Physics/PhysModel.hpp"
#include "Physics/Variables.hpp"

#include "RDM/Core/FaceLoop.hpp"
#include "RDM/Core/BoundaryTerm.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;

namespace CF {
namespace RDM {

/////////////////////////////////////////////////////////////////////////////////////

BoundaryTerm::BoundaryTerm ( const std::string& name ) :
  Solver::Action(name)
{
  mark_basic();

  signal("create_component")->hidden(true);
  signal("rename_component")->hidden(true);
  signal("delete_component")->hidden(true);
  signal("move_component"  )->hidden(true);
}

BoundaryTerm::~BoundaryTerm() {}

ElementLoop& BoundaryTerm::access_element_loop( const std::string& type_name )
{
  const std::string update_vars_type =
      physical_model().get_child( RDM::Tags::update_vars() )
                      .as_type<Physics::Variables>()
                      .type();

  // get the element loop or create it if does not exist
  ElementLoop::Ptr loop;
  Common::Component::Ptr cloop = get_child_ptr( "LOOP" );
  if( is_null( cloop ) )
  {
    loop = build_component_abstract_type_reduced< FaceLoop >( "FaceLoopT<" + type_name + "," + update_vars_type + ">" , "LOOP");
    add_component(loop);
  }
  else
    loop = cloop->as_ptr_checked<ElementLoop>();


  return *loop;
}


/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////
