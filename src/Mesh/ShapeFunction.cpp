// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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

RealRowVector ShapeFunction::value(const RealVector& local_coord) const
{
  throw Common::NotImplemented(FromHere(),"shape function value not implemented for " + derived_type_name());
  return RealRowVector(1);
}

////////////////////////////////////////////////////////////////////////////////

void ShapeFunction::compute_value(const RealVector& local_coordinate, RealRowVector& value) const
{
  throw Common::NotImplemented(FromHere(),"compute_value not implemented for " + derived_type_name());
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix ShapeFunction::gradient(const RealVector& local_coord) const
{
  throw Common::NotImplemented(FromHere(),"shape function gradient not implemented for " + derived_type_name());
  return RealRowVector(0,0);
}

////////////////////////////////////////////////////////////////////////////////

void ShapeFunction::compute_gradient(const RealVector& local_coordinate, RealMatrix& gradient) const
{
  throw Common::NotImplemented(FromHere(),"compute_gradient not implemented for " + derived_type_name());
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& ShapeFunction::local_coordinates() const
{
  throw Common::NotImplemented(FromHere(),"local_coordinates not implemented for " + derived_type_name());
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
