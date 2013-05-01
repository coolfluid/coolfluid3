// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_LagrangeP0_Quad_hpp
#define cf3_mesh_LagrangeP0_Quad_hpp

#include "mesh/ShapeFunctionBase.hpp"
#include "mesh/LagrangeP0/API.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP0 {

////////////////////////////////////////////////////////////////////////////////

struct Mesh_LagrangeP0_API Quad_traits
{
  enum { nb_nodes       = 1               };
  enum { nb_faces       = 4               };
  enum { dimensionality = 2               };
  enum { order          = 0               };
  enum { shape          = GeoShape::QUAD  };
};

////////////////////////////////////////////////////////////////////////////////

/// @class Quad
/// @verbatim
/// Local connectivity:
///             -------------
///             |           |
///             |           |
///             |     0     |
///             |           |
///             |           |
///             -------------
/// Reference domain: <-1,1> x <-1,1>
/// @endverbatim
/// @see ShapeFunction for documentation on undocumented static functions
struct Mesh_LagrangeP0_API Quad : public ShapeFunctionBase<Quad,Quad_traits>
{
  static const RealMatrix& local_coordinates();
  static void compute_value(const MappedCoordsT& mapped_coord, ValueT& result);
  static void compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result);
};

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP0
} // mesh
} // cf3

#endif // cf3_mesh_LagrangeP0_Quad_hpp
