// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/Foreach.hpp"
#include "common/URI.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"
#include "common/Signal.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"

#include "physics/PhysModel.hpp"

#include "solver/LibSolver.hpp"
#include "solver/Time.hpp"
#include "solver/ActionDirectorWithSkip.hpp"
#include "solver/Solver.hpp"
#include "solver/Tags.hpp"

namespace cf3 {
  common::ComponentBuilder < solver::ActionDirectorWithSkip, common::ActionDirector, solver::LibSolver > SolverActionDirectorWithSkip_Builder;
namespace solver {


using namespace cf3::common;
using namespace cf3::mesh;
  
////////////////////////////////////////////////////////////////////////////////////////////

ActionDirectorWithSkip::ActionDirectorWithSkip ( const std::string& name ) :
  solver::ActionDirector(name),
  m_interval(1),
  m_count(0)
{
  options().add("interval", m_interval)
    .pretty_name("Interval")
    .description("Execute once every interval")
    .mark_basic()
    .link_to(&m_interval);
}

ActionDirectorWithSkip::~ActionDirectorWithSkip() {}

void ActionDirectorWithSkip::execute()
{
  if((m_count % m_interval) == 0)
  {
    solver::ActionDirector::execute();
  }
  ++m_count;
}



////////////////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3
