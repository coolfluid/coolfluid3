// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/OptionComponent.hpp"
#include "common/CBuilder.hpp"
#include "common/FindComponents.hpp"

#include "mesh/Field.hpp"
#include "mesh/CMesh.hpp"
#include "mesh/MeshMetadata.hpp"

#include "Solver/Tags.hpp"
#include "Solver/CModel.hpp"
#include "Solver/CTime.hpp"
#include "Solver/Actions/CAdvanceTime.hpp"

namespace cf3 {
namespace Solver {
namespace Actions {

using namespace common;
using namespace mesh;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CAdvanceTime, CAction, LibActions > CAdvanceTime_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

CAdvanceTime::CAdvanceTime( const std::string& name  ) :
  Solver::Action ( name )
{
  mark_basic();

  properties()["brief"] = std::string("Time advancing object");
  properties()["description"] = std::string( "This object handles time advancing\n" );

  options().add_option( OptionComponent<CTime>::create(Solver::Tags::time(), &m_time))
      ->description("Time tracking component")
      ->pretty_name("Time")
      ->mark_basic();
}


CAdvanceTime::~CAdvanceTime()  {}



CTime& CAdvanceTime::time()
{
  CTime::Ptr t = m_time.lock();
  if( is_null(t) )
    throw common::SetupError( FromHere(),
                              "Time not yet set for component " + uri().string() );
  return *t;
}


void CAdvanceTime::execute ()
{
  time().current_time() += time().dt();
  time().iter() += 1u;

  mesh().metadata()["time"] = time().current_time();
  mesh().metadata()["iter"] = time().iter();
}

////////////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // cf3
