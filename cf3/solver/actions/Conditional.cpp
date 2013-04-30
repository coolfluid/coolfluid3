// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @todo remove
#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"
#include "common/PropertyList.hpp"

#include "solver/actions/Conditional.hpp"
#include "solver/Criterion.hpp"

namespace cf3 {
namespace solver {
namespace actions {

using namespace common;

common::ComponentBuilder < Conditional, common::Action, LibActions > Conditional_Builder;

////////////////////////////////////////////////////////////////////////////////

Conditional::Conditional( const std::string& name  ) :
  Action ( name )
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
  boost_foreach(Criterion& if_criterion, find_components<Criterion>(*this))
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
    boost_foreach(Action& action, find_components<Action>(*this))
    {
      action.execute();
    }
  }

}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3
