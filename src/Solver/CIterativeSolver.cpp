// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ObjectProvider.hpp"

#include "Solver/LibSolver.hpp"

#include "Solver/CIterativeSolver.hpp"

namespace CF {
namespace Solver {

using namespace Common;
using namespace Common::String;

Common::ObjectProvider < CIterativeSolver, Component, LibSolver, NB_ARGS_1 >
CIterativeSolver_Provider ( CIterativeSolver::type_name() );

////////////////////////////////////////////////////////////////////////////////

CIterativeSolver::CIterativeSolver ( const CName& name  ) :
  CMethod ( name )
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////

CIterativeSolver::~CIterativeSolver()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
