// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "Solver/LibSolver.hpp"

namespace CF {
namespace Solver {

CF::Common::RegistLibrary<LibSolver> libSolver;

////////////////////////////////////////////////////////////////////////////////

void LibSolver::initiate()
{
}

void LibSolver::terminate()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
