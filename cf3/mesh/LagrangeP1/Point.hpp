// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_LagrangeP1_Point_hpp
#define cf3_mesh_LagrangeP1_Point_hpp

#include "mesh/ShapeFunctionBase.hpp"
#include "mesh/LagrangeP1/API.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

struct Mesh_LagrangeP1_API Point_traits
{
  enum { nb_nodes       = 1               };
  enum { nb_faces       = 0               };
  enum { dimensionality = 0               };
  enum { order          = 1               };
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
struct Mesh_LagrangeP1_API Point
{
  // Information coming from Traits
  // ------------------------------
  static const GeoShape::Type shape          = (GeoShape::Type) Point_traits::shape;
  static const Uint           nb_nodes       = (Uint) Point_traits::nb_nodes;
  static const Uint           nb_faces       = (Uint) Point_traits::nb_faces;
  static const Uint           dimensionality = (Uint) Point_traits::dimensionality;
  static const Uint           order          = (Uint) Point_traits::order;

  static std::string type_name() { return GeoShape::Convert::instance().to_str(shape); }

  // Typedefs for special matrices
  // -----------------------------
  typedef Eigen::Matrix<Real, 3, 1       >  MappedCoordsT;
  typedef Eigen::Matrix<Real, 1, nb_nodes>  ValueT;
  typedef Eigen::Matrix<Real, 3, nb_nodes>  GradientT;

  // Not-implemented static functions
  // --------------------------------
  static const RealMatrix& local_coordinates();
  static const RealMatrix& face_normals();
  static void compute_value(const MappedCoordsT& mapped_coord, ValueT& result);
  static void compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result);

  // Implemented static functions
  // ----------------------------
  static ValueT value(const MappedCoordsT& mapped_coord)
  {
    ValueT result;
    compute_value(mapped_coord,result);
    return result;
  }

  static GradientT gradient(const MappedCoordsT& mapped_coord)
  {
    GradientT result;
    compute_gradient(mapped_coord,result);
    return result;
  }
};

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // mesh
} // cf3

#endif // cf3_mesh_LagrangeP1_Point_hpp
