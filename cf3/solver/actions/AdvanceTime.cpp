// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"

#include "mesh/Field.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/MeshMetadata.hpp"

#include "solver/Tags.hpp"
#include "solver/Model.hpp"
#include "solver/Time.hpp"
#include "solver/actions/AdvanceTime.hpp"

namespace cf3 {
namespace solver {
namespace actions {

using namespace common;
using namespace mesh;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < AdvanceTime, common::Action, LibActions > AdvanceTime_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

AdvanceTime::AdvanceTime( const std::string& name  ) :
  solver::Action ( name )
{
  mark_basic();

  properties()["brief"] = std::string("Time advancing object");
  properties()["description"] = std::string( "This object handles time advancing\n" );

  options().add_option(solver::Tags::time(), m_time)
      .description("Time tracking component")
      .pretty_name("Time")
      .mark_basic()
      .link_to(&m_time);
}


AdvanceTime::~AdvanceTime()  {}



Time& AdvanceTime::time()
{
  Handle< Time > t = m_time;
  if( is_null(t) )
    throw common::SetupError( FromHere(),
                              "Time not yet set for component " + uri().string() );
  return *t;
}


void AdvanceTime::execute ()
{
  Time& time = this->time();
  time.options().configure_option("iteration", time.iter() + 1);
  time.options().configure_option("current_time", time.dt() * static_cast<Real>(time.iter()));

  // TODO: this should really be handled in another way. Needing to configure a time advancement action with a mesh is too much hassle
  if(is_not_null(m_mesh))
  {
    mesh().metadata()["time"] = time.current_time();
    mesh().metadata()["iter"] = time.iter();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3
