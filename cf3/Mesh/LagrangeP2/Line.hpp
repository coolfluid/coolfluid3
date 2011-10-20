// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Mesh_LagrangeP2_Line_hpp
#define cf3_Mesh_LagrangeP2_Line_hpp

#include "Mesh/ShapeFunctionBase.hpp"
#include "Mesh/LagrangeP2/API.hpp"

namespace cf3 {
namespace Mesh {
namespace LagrangeP2 {

////////////////////////////////////////////////////////////////////////////////

struct Mesh_LagrangeP2_API Line_traits
{
  enum { nb_nodes       = 3               };
  enum { dimensionality = 1               };
  enum { order          = 2               };
  enum { shape          = GeoShape::LINE  };
};

////////////////////////////////////////////////////////////////////////////////

/// @class Line
/// @verbatim
/// Local connectivity:
///             0-----2-----1
/// Reference domain: <-1,1>
/// @endverbatim
/// @see ShapeFunction for documentation on undocumented static functions
struct Mesh_LagrangeP2_API Line : ShapeFunctionBase<Line,Line_traits>
{
  static const RealMatrix& local_coordinates();
  static void compute_value(const MappedCoordsT& mapped_coord, ValueT& result);
  static void compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result);
};

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP2
} // Mesh
} // cf3

#endif // cf3_Mesh_LagrangeP2_Line_hpp
