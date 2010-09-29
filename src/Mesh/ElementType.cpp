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

ElementType::ElementType()
{
}

ElementType::~ElementType()
{
}

Real ElementType::jacobian_determinantV( const CF::RealVector& mapped_coord, const CF::Mesh::ElementType::NodesT& nodes) const
{
  throw Common::NotImplemented(FromHere(), "Shape function " + getElementTypeName() + " does not support jacobian_determinants");
}


////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF
