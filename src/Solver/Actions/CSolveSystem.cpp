// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/OptionComponent.hpp"
#include "Common/CBuilder.hpp"

#include "Solver/CEigenLSS.hpp"

#include "CSolveSystem.hpp"

namespace CF {
namespace Solver {
namespace Actions {

using namespace Common;
using namespace Mesh;

Common::ComponentBuilder < CSolveSystem, CAction, LibActions > CSolveSystem_Builder;

////////////////////////////////////////////////////////////////////////////////

CSolveSystem::CSolveSystem( const std::string& name  ) :
  CAction ( name )
{
  mark_basic();

  properties()["brief"] = std::string("Runs a linear system solver");
  std::string description =
    "This object executes a linear system solver\n";
  properties()["description"] = description;
  
  options().add_option( OptionComponent<CEigenLSS>::create("lss", "LSS", "Linear System solver that gets executed",
                                                                   &m_lss))
    ->mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void CSolveSystem::execute ()
{
  if(m_lss.expired())
    throw SetupError(FromHere(), "LSS not set for component " + uri().string());
  
  CEigenLSS& lss = *m_lss.lock();
  
  if(!lss.size())
    throw SetupError(FromHere(), "LSS at " + lss.uri().string() + " is empty!");
  
  lss.solve();
}

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF
