// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/PropertyList.hpp"

#include "solver/Wizard.hpp"

namespace cf3 {
namespace solver {

using namespace common;

////////////////////////////////////////////////////////////////////////////////

Wizard::Wizard ( const std::string& name  ) :
  Component ( name )
{
  mark_basic();

  // properties

  properties()["brief"] = std::string("Wizard");
  properties()["description"] = std::string("");
}

////////////////////////////////////////////////////////////////////////////////

Wizard::~Wizard()
{
}

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3
