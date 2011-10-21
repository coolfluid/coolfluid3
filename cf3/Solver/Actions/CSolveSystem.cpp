// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/OptionComponent.hpp"
#include "common/CBuilder.hpp"

#include "math/LSS/System.hpp"

#include "CSolveSystem.hpp"

namespace cf3 {
namespace Solver {
namespace Actions {

using namespace common;
using namespace math;

common::ComponentBuilder < CSolveSystem, common::Action, LibActions > CSolveSystem_Builder;

////////////////////////////////////////////////////////////////////////////////

CSolveSystem::CSolveSystem( const std::string& name  ) :
  Action ( name )
{
  mark_basic();

  properties()["brief"] = std::string("Runs a linear system solver");
  std::string description =
    "This object executes a linear system solver\n";
  properties()["description"] = description;

  options().add_option( OptionComponent<LSS::System>::create("lss", &m_lss))
      ->description("Linear System solver that gets executed")
      ->pretty_name("LSS")
      ->mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void CSolveSystem::execute ()
{
  if(m_lss.expired())
    throw SetupError(FromHere(), "LSS not set for component " + uri().string());

  LSS::System& lss = *m_lss.lock();

  if(!lss.is_created())
    throw SetupError(FromHere(), "LSS at " + lss.uri().string() + " is not created!");

  lss.solve();
}

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // cf3
