// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CFactories.hpp"
#include "Common/Signal.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

CFactories::CFactories ( const std::string& name) : Component ( name )
{
  m_properties["brief"] = std::string("Factories");
  std::string description =
    "Stores all Component Builders.\n"
    "Builders can be accessed in advanced mode, to build components\n";
  m_properties["description"] = description;
  
  signal("create_component")->hidden(true);
  signal("rename_component")->hidden(true);
  signal("delete_component")->hidden(true);
  signal("move_component")->hidden(true);
}

////////////////////////////////////////////////////////////////////////////////

CFactories::~CFactories()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
