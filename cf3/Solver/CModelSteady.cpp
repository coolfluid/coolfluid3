// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"

#include "Solver/CModelSteady.hpp"
#include "Solver/CSolver.hpp"

namespace cf3 {
namespace Solver {

using namespace common;

common::ComponentBuilder < CModelSteady, Component, LibSolver > CModelSteady_Builder;

////////////////////////////////////////////////////////////////////////////////

CModelSteady::CModelSteady( const std::string& name  ) :
  CModel ( name )
{
   m_properties["steady"] = bool(true);
}

////////////////////////////////////////////////////////////////////////////////

CModelSteady::~CModelSteady()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // cf3
