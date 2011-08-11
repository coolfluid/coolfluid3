// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @todo remove
#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/Foreach.hpp"
#include "Common/FindComponents.hpp"

#include "Solver/Actions/Conditional.hpp"
#include "Solver/Actions/CCriterion.hpp"

namespace CF {
namespace Solver {
namespace Actions {

using namespace Common;

Common::ComponentBuilder < Conditional, CAction, LibActions > Conditional_Builder;

////////////////////////////////////////////////////////////////////////////////

Conditional::Conditional( const std::string& name  ) :
  CAction ( name )
{
  mark_basic();
  properties()["brief"] = std::string("Iterator object");
  std::string description =
  "This object handles a conditional action\n"
  "It can have one or more \"if\" criteria. The critera are linked with \"OR\". \n"
  "Merge criteria yourself if you want to link \"AND\" or combinations.";
  properties()["description"] = description;


}

////////////////////////////////////////////////////////////////////////////////

Conditional::~Conditional()
{
}

////////////////////////////////////////////////////////////////////////////////

void Conditional::execute ()
{
  bool conditional = true;
  // check if any criterion are met and abort if so
  boost_foreach(CCriterion& if_criterion, find_components<CCriterion>(*this))
  {
    conditional = false;
    if (if_criterion())
    {
      conditional = true;
      break;
    }
  }

  if (conditional)
  {
    boost_foreach(CAction& action, find_components<CAction>(*this))
    {
      action.execute();
    }
  }

}

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF
