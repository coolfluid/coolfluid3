// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_LagrangeP2_Quad_hpp
#define CF_Mesh_LagrangeP2_Quad_hpp

#include "Mesh/ShapeFunctionBase.hpp"
#include "Mesh/LagrangeP2/API.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeP2 {

////////////////////////////////////////////////////////////////////////////////

struct Mesh_LagrangeP2_API Quad_traits
{
  enum { nb_nodes       = 9               };
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
} // Mesh
} // CF

#endif // CF_Mesh_LagrangeP2_Quad_hpp
