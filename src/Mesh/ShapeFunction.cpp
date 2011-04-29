// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Mesh/ShapeFunction.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

ShapeFunction::ShapeFunction( const std::string& name ) : Common::Component(name)
{   
}

////////////////////////////////////////////////////////////////////////////////

ShapeFunction::~ShapeFunction()
{
}

////////////////////////////////////////////////////////////////////////////////

void ShapeFunction::compute_value(const RealVector& local_coord, RealRowVector& result)
{
  throw Common::NotImplemented(FromHere(),"shape function value not implemented for" + derived_type_name());
}

////////////////////////////////////////////////////////////////////////////////

void ShapeFunction::compute_gradient(const RealVector& local_coord, RealMatrix& result)
{
  throw Common::NotImplemented(FromHere(),"shape function gradient not implemented for" + derived_type_name());
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
