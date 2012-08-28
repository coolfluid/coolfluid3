// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Signal.hpp"
#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/EventHandler.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionComponent.hpp"
#include "common/ActionDirector.hpp"
#include "common/FindComponents.hpp"
#include "common/Group.hpp"
#include "common/Action.hpp"

#include "math/VariablesDescriptor.hpp"

#include "solver/Time.hpp"

#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"

#include "sdm/ExplicitSolver.hpp"
#include "sdm/Tags.hpp"

#include "solver/History.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::mesh;

namespace cf3 {
namespace sdm {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < sdm::ExplicitSolver, common::Action, LibSDM > explicitSolver_Builder;

///////////////////////////////////////////////////////////////////////////////////////

ExplicitSolver::ExplicitSolver( const std::string& name ) :
  sdm::Solver(name)
{
  mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void ExplicitSolver::setup()
{
  m_time_integration->options().set("pre_update",m_pre_update);
  m_time_integration->options().set("post_update",m_post_update);
}

////////////////////////////////////////////////////////////////////////////////

void ExplicitSolver::step()
{
  m_time_integration->handle<Action>()->execute();
}

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3
