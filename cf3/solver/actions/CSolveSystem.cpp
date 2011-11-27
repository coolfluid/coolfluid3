// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/Builder.hpp"

#include "math/LSS/System.hpp"

#include "CSolveSystem.hpp"

namespace cf3 {
namespace solver {
namespace actions {

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

  options().add_option("lss", &m_lss)
      .description("Linear System solver that gets executed")
      .pretty_name("LSS")
      .mark_basic()
      .link_to(&m_lss);
}

////////////////////////////////////////////////////////////////////////////////

void CSolveSystem::execute ()
{
  if(is_null(m_lss))
    throw SetupError(FromHere(), "LSS not set for component " + uri().string());

  LSS::System& lss = *m_lss;

  if(!lss.is_created())
    throw SetupError(FromHere(), "LSS at " + lss.uri().string() + " is not created!");

  lss.solve();
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3
