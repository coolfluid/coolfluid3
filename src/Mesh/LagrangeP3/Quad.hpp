// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_LagrangeP3_Quad_hpp
#define CF_Mesh_LagrangeP3_Quad_hpp

#include "Mesh/ShapeFunctionBase.hpp"
#include "Mesh/LagrangeP3/API.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeP3 {

////////////////////////////////////////////////////////////////////////////////

struct Mesh_LagrangeP3_API Quad_traits
{
  enum { nb_nodes       = 16              };
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
} // Mesh
} // CF

#endif // CF_Mesh_LagrangeP3_Quad_hpp
