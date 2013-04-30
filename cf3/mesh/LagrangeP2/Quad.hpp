// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_LagrangeP2_Quad_hpp
#define cf3_mesh_LagrangeP2_Quad_hpp

#include "mesh/ShapeFunctionBase.hpp"
#include "mesh/LagrangeP2/API.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP2 {

////////////////////////////////////////////////////////////////////////////////

struct Mesh_LagrangeP2_API Quad_traits
{
  enum { nb_nodes       = 9               };
  enum { nb_faces       = 4               };
  enum { dimensionality = 2               };
  enum { order          = 2               };
  enum { shape          = GeoShape::QUAD  };
};

////////////////////////////////////////////////////////////////////////////////

/// @class Quad
/// @verbatim
/// Local connectivity:
///             3-----6-----2
///             |           |
///             |           |
///             7     8     5
///             |           |
///             |           |
///             0-----4-----1
/// Reference domain: <-1,1> x <-1,1>
/// @endverbatim
/// @see ShapeFunction for documentation on undocumented static functions
struct Mesh_LagrangeP2_API Quad : public ShapeFunctionBase<Quad,Quad_traits>
{
  static const RealMatrix& local_coordinates();
  static void compute_value(const MappedCoordsT& mapped_coord, ValueT& result);
  static void compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result);
};

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP2
} // mesh
} // cf3

#endif // cf3_mesh_LagrangeP2_Quad_hpp
