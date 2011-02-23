// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/Foreach.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Solver/CModelSteady.hpp"
#include "Solver/CSolver.hpp"

namespace CF {
namespace Solver {

using namespace Common;

Common::ComponentBuilder < CModelSteady, Component, LibSolver > CModelSteady_Builder;

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

void CModelSteady::simulate ()
{
  boost_foreach(CSolver& is, find_components<CSolver>(*this))
    is.solve();
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
