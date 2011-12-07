// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"
#include "common/PropertyList.hpp"

#include "solver/CModelSteady.hpp"
#include "solver/CSolver.hpp"

namespace cf3 {
namespace solver {

using namespace common;

common::ComponentBuilder < CModelSteady, Component, LibSolver > CModelSteady_Builder;

////////////////////////////////////////////////////////////////////////////////

CModelSteady::CModelSteady( const std::string& name  ) :
  CModel ( name )
{
   properties()["steady"] = bool(true);
}

////////////////////////////////////////////////////////////////////////////////

CModelSteady::~CModelSteady()
{
}

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3
