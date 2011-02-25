// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Solver/CWizard.hpp"

namespace CF {
namespace Solver {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CWizard::CWizard ( const std::string& name  ) :
  Component ( name )
{
  mark_basic();

  // properties

  properties()["brief"] = std::string("Wizard");
  properties()["description"] = std::string("");
}

////////////////////////////////////////////////////////////////////////////////

CWizard::~CWizard()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
