// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Solver/CDomain.hpp"

namespace CF {
namespace Solver {

using namespace Common;
using namespace Common::String;

Common::ComponentBuilder < CDomain, Component, LibSolver > CDomain_Builder;

////////////////////////////////////////////////////////////////////////////////

CDomain::CDomain( const std::string& name  ) : Component ( name )
{
   
}

////////////////////////////////////////////////////////////////////////////////

CDomain::~CDomain()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
