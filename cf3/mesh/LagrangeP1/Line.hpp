// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_LagrangeP1_Line_hpp
#define cf3_mesh_LagrangeP1_Line_hpp

#include "mesh/ShapeFunctionBase.hpp"
#include "mesh/LagrangeP1/API.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

struct Mesh_LagrangeP1_API Line_traits
{
  enum { nb_nodes       = 2               };
  enum { nb_faces       = 2               };
  enum { dimensionality = 1               };
  enum { order          = 1               };
  enum { shape          = GeoShape::LINE  };
};

////////////////////////////////////////////////////////////////////////////////

/// @class Line
/// @verbatim
/// Local connectivity:
///             0-----------1
/// Reference domain: <-1,1>
/// @endverbatim
/// @see ShapeFunction for documentation on undocumented static functions
struct Mesh_LagrangeP1_API Line : ShapeFunctionBase<Line,Line_traits>
{
  static const RealMatrix& local_coordinates();
  static void compute_value(const MappedCoordsT& mapped_coord, ValueT& result);
  static void compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result);
};

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // mesh
} // cf3

#endif // cf3_mesh_LagrangeP1_Line_hpp
