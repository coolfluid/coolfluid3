// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Factories.hpp"
#include "common/Signal.hpp"
#include "common/PropertyList.hpp"

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

Factories::Factories ( const std::string& name) : Component ( name )
{
  properties()["brief"] = std::string("Factories");
  std::string description =
    "Stores all Component Builders.\n"
    "Builders can be accessed in advanced mode, to build components\n";
  properties()["description"] = description;

  signal("create_component")->hidden(true);
  signal("rename_component")->hidden(true);
  signal("delete_component")->hidden(true);
  signal("move_component")->hidden(true);
}

////////////////////////////////////////////////////////////////////////////////

Factories::~Factories()
{
}

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
