// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "solver/Actions/CCriterion.hpp"

namespace cf3 {
namespace solver {
namespace Actions {

using namespace common;

////////////////////////////////////////////////////////////////////////////////

CCriterion::CCriterion( const std::string& name  ) :
  Component ( name )
{
  mark_basic();
  properties()["brief"] = std::string("Criterion object");
  std::string description =
  "This object handles implements the round bracket operator and returns\n"
  "true if the criterion is met\n";
  properties()["description"] = description;
  
}

////////////////////////////////////////////////////////////////////////////////

CCriterion::~CCriterion()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // solver
} // cf3
