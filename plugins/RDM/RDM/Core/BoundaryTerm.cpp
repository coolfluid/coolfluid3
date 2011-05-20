// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Signal.hpp"

#include "RDM/Core/BoundaryTerm.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;

namespace CF {
namespace RDM {

/////////////////////////////////////////////////////////////////////////////////////

BoundaryTerm::BoundaryTerm ( const std::string& name ) :
  Solver::Action(name)
{
  mark_basic();

  signal("create_component")->is_hidden = true;
  signal("rename_component")->is_hidden = true;
  signal("delete_component")->is_hidden = true;
  signal("move_component"  )->is_hidden = true;
}

BoundaryTerm::~BoundaryTerm()
{
}

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////
