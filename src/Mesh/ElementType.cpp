// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Mesh/ElementType.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

ElementType::ElementType( const std::string& name ) : Common::Component(name)
{
  tag_component(this);
}

ElementType::~ElementType()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
