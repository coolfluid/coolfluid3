// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_LagrangeP3_Quad_hpp
#define cf3_mesh_LagrangeP3_Quad_hpp

#include "mesh/ShapeFunctionBase.hpp"
#include "mesh/LagrangeP3/API.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP3 {

////////////////////////////////////////////////////////////////////////////////

struct Mesh_LagrangeP3_API Quad_traits
{
  enum { nb_nodes       = 16              };
  enum { nb_faces       = 4               };
  enum { dimensionality = 2               };
  enum { order          = 3               };
  enum { shape          = GeoShape::QUAD  };
};

////////////////////////////////////////////////////////////////////////////////

/// @class Quad
/// @verbatim
/// Local connectivity:
///             3---9---8---2
///             |           |
///            10  15   14  7
///             |           |
///            11  12   13  6
///             |           |
///             0---4---5---1
/// Reference domain: <-1,1> x <-1,1>
/// @endverbatim
/// @see ShapeFunction for documentation on undocumented static functions
struct Mesh_LagrangeP3_API Quad : public ShapeFunctionBase<Quad,Quad_traits>
{
  static const RealMatrix& local_coordinates();
  static void compute_value(const MappedCoordsT& mapped_coord, ValueT& result);
  static void compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result);
};

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP3
} // mesh
} // cf3

#endif // cf3_mesh_LagrangeP3_Quad_hpp
