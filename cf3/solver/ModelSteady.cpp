// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"
#include "common/PropertyList.hpp"

#include "solver/ModelSteady.hpp"
#include "solver/Solver.hpp"

namespace cf3 {
namespace solver {

using namespace common;

common::ComponentBuilder < ModelSteady, Component, LibSolver > ModelSteady_Builder;

////////////////////////////////////////////////////////////////////////////////

ModelSteady::ModelSteady( const std::string& name  ) :
  Model ( name )
{
   properties()["steady"] = bool(true);
}

////////////////////////////////////////////////////////////////////////////////

ModelSteady::~ModelSteady()
{
}

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3
