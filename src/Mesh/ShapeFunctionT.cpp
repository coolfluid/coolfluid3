// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Mesh/ShapeFunctionT.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& ShapeFunctionFallBack::local_coordinates()
{
  throw Common::NotImplemented(FromHere(),"static function SF::local_coordinates not implemented or not applicable.\n Check backtrace to see which shape function this is about.");
  const static RealMatrix real_matrix_obj;
  return real_matrix_obj;
}

ShapeFunctionFallBack::ValueT ShapeFunctionFallBack::value(const MappedCoordsT& mapped_coord)
{
  throw Common::NotImplemented(FromHere(),"static function SF::value not implemented or not applicable.\n Check backtrace to see which shape function this is about.");
  return ValueT();
}

////////////////////////////////////////////////////////////////////////////////

ShapeFunctionFallBack::GradientT ShapeFunctionFallBack::gradient(const MappedCoordsT& mapped_coord)
{
  throw Common::NotImplemented(FromHere(),"static function SF::gradient not implemented or not applicable.\n Check backtrace to see which shape function this is about.");
  return GradientT();
}

////////////////////////////////////////////////////////////////////////////////

void ShapeFunctionFallBack::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  throw Common::NotImplemented(FromHere(),"static function SF::compute_value not implemented or not applicable.\n Check backtrace to see which shape function this is about.");
}

////////////////////////////////////////////////////////////////////////////////

void ShapeFunctionFallBack::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  throw Common::NotImplemented(FromHere(),"static function SF::compute_gradient not implemented or not applicable.\n Check backtrace to see which shape function this is about.");
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
