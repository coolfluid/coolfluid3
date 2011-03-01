// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Solver/LibSolver.hpp"
#include "Solver/CMethod.hpp"

namespace CF {
namespace Solver {

using namespace Common;

Common::ComponentBuilder < CMethod, Component, LibSolver > CMethod_Builder;

////////////////////////////////////////////////////////////////////////////////

CMethod::CMethod ( const std::string& name  ) :
  Component ( name )
{
  // signals
  regist_signal ( "run_operation" , "run an operation", "Run Operation" )->connect ( boost::bind ( &CMethod::run_operation, this, _1 ) );
}

////////////////////////////////////////////////////////////////////////////////

CMethod::~CMethod() {}

////////////////////////////////////////////////////////////////////////////////

CMethod& CMethod::operation(const std::string& name)
{
  return *get_child_ptr(name)->as_ptr<CMethod>();
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
