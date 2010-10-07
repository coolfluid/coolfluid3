// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ObjectProvider.hpp"

#include "Solver/LibSolver.hpp"

#include "Solver/CDiscretization.hpp"

namespace CF {
namespace Solver {

using namespace Common;
using namespace Common::String;

Common::ObjectProvider < CDiscretization, Component, LibSolver, NB_ARGS_1 >
CDiscretization_Provider ( CDiscretization::type_name() );

////////////////////////////////////////////////////////////////////////////////

CDiscretization::CDiscretization ( const CName& name  ) :
  Component ( name )
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////

CDiscretization::~CDiscretization()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
