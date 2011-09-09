// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_LagrangeP3_Triag_hpp
#define CF_Mesh_LagrangeP3_Triag_hpp

#include "Mesh/ShapeFunctionBase.hpp"
#include "Mesh/LagrangeP3/API.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeP3 {

////////////////////////////////////////////////////////////////////////////////

struct Mesh_LagrangeP3_API Triag_traits
{
  enum { nb_nodes       = 10              };
  enum { dimensionality = 2               };
  enum { order          = 3               };
  enum { shape          = GeoShape::TRIAG };
};

////////////////////////////////////////////////////////////////////////////////

/// @class Triag
/// @verbatim
/// Local connectivity:
///             2
///             | .
///             7   6
///             |     .
///             8   9   5
///             |         .
///             0---3---4---1
/// Reference domain: <0,1> x <0,1>
/// @endverbatim
/// @see ShapeFunction for documentation on undocumented static functions
struct Mesh_LagrangeP3_API Triag : public ShapeFunctionBase<Triag,Triag_traits>
{
  static const RealMatrix& local_coordinates();
  static void compute_value(const MappedCoordsT& mapped_coord, ValueT& result);
  static void compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result);
};

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP3
} // Mesh
} // CF

#endif // CF_Mesh_LagrangeP3_Triag_hpp
