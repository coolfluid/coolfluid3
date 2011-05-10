// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "SFDM/ShapeFunction.hpp"

namespace CF {
namespace SFDM {

////////////////////////////////////////////////////////////////////////////////

ShapeFunction::ShapeFunction(const std::string& name) : Mesh::ShapeFunction(name)
{
}

////////////////////////////////////////////////////////////////////////////////

const ShapeFunction& ShapeFunction::line() const
{
  throw Common::NotImplemented(FromHere(),"line() not implemented for "+derived_type_name() );
  static const ShapeFunction f;
  return f;
}

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF
