// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/OptionComponent.hpp"
#include "Common/CBuilder.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/Field.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/MeshMetadata.hpp"

#include "Solver/Tags.hpp"
#include "Solver/CModel.hpp"
#include "Solver/CTime.hpp"
#include "Solver/Actions/CAdvanceTime.hpp"

namespace CF {
namespace Solver {
namespace Actions {

using namespace Common;
using namespace Mesh;

////////////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CAdvanceTime, CAction, LibActions > CAdvanceTime_Builder;

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
    throw Common::SetupError( FromHere(),
                              "Time not yet set for component " + uri().string() );
  return *t;
}


void CAdvanceTime::execute ()
{
  time().current_time() += time().dt();
  time().iter() += 1u;

  mesh().metadata()["time"] = time().current_time();
  mesh().metadata()["iter"] = time().iter();

  boost_foreach(Field& field, find_components_recursively<Field>(mesh()))
  {
    field.configure_option("time",time().current_time());
    field.configure_option("iteration", time().iter());
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF
