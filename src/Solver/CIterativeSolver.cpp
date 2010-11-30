// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Solver/CIterativeSolver.hpp"

namespace CF {
namespace Solver {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CIterativeSolver::CIterativeSolver ( const std::string& name  ) :
  CMethod ( name )
{
   
  
  properties()["brief"]=std::string("Iterative Solver component");
  properties()["description"]=std::string("Handles time stepping and convergence operations");
}

////////////////////////////////////////////////////////////////////////////////

CIterativeSolver::~CIterativeSolver()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
