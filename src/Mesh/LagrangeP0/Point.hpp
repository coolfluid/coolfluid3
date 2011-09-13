// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_LagrangeP0_Point_hpp
#define CF_Mesh_LagrangeP0_Point_hpp

#include "Mesh/ShapeFunctionBase.hpp"
#include "Mesh/LagrangeP0/API.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeP0 {

////////////////////////////////////////////////////////////////////////////////

struct Mesh_LagrangeP0_API Point_traits
{
  enum { nb_nodes       = 1               };
  enum { dimensionality = 0               };
  enum { order          = 0               };
  enum { shape          = GeoShape::POINT };
};

/// @class Point
/// @verbatim
/// Local connectivity:
///
///      0
/// Reference domain: <0,0>
/// @endverbatim
/// @see ShapeFunction for documentation on undocumented static functions
struct Mesh_LagrangeP0_API Point : ShapeFunctionBase<Point,Point_traits>
{
  static const RealMatrix& local_coordinates();
  static void compute_value(const MappedCoordsT& mapped_coord, ValueT& result);
  static void compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result);
};

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP0
} // Mesh
} // CF

#endif // CF_Mesh_LagrangeP0_Point_hpp
