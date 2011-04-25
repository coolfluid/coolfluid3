// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SF_SFLineLagrangeP1_hpp
#define CF_Mesh_SF_SFLineLagrangeP1_hpp

#include "Mesh/ShapeFunction.hpp"
#include "Mesh/GeoShape.hpp"
#include "Mesh/SF/LibSF.hpp"

namespace CF {
namespace Mesh {
namespace SF {

class MESH_SF_API SFLineLagrangeP1  : public ShapeFunction {
public:

  static const Uint dimensionality = 1;
  static const Uint nb_nodes = 2;
  static const Uint order = 1;
  static const GeoShape::Type shape = GeoShape::LINE;

public:

  /// Constructor
  SFLineLagrangeP1(const std::string& name = type_name());

  /// Type name
  static std::string type_name() { return "SFLineLagrangeP1"; }

  /// Types for the matrices used
  typedef Eigen::Matrix<Real, dimensionality, 1> MappedCoords_t;
  typedef Eigen::Matrix<Real, 1, nb_nodes> ShapeFunction_t;
  typedef Eigen::Matrix<Real, dimensionality, nb_nodes> ShapeFunctionGradient_t;
  typedef Eigen::Matrix<Real, nb_nodes, dimensionality> MappedNodes_t;

  /// Compute the shape functions corresponding to the given
  /// mapped coordinates
  /// @param mappedCoord The mapped coordinates
  /// @param shapeFunc Vector storing the result
  static void shape_function(const MappedCoords_t& mapped_coord, ShapeFunction_t& shape_func);

  /// Compute the gradient with respect to mapped coordinates, i.e. parial derivatives are in terms of the
  /// mapped coordinates. The result needs to be multiplied with the inverse jacobian to get the result in real
  /// coordinates.
  /// @param mappedCoord The mapped coordinates where the gradient should be calculated
  /// @param result Storage for the resulting gradient matrix
  static void shape_function_gradient(const MappedCoords_t& mapped_coord, ShapeFunctionGradient_t& result);

  /// Coordinates in mapped space of the nodes defining the shape function (nb_nodes x dimensionality)
  static const MappedNodes_t& mapped_sf_nodes() { return s_mapped_sf_nodes; }

private:
  
  static MappedNodes_t s_mapped_sf_nodes;
  
};

} // SF
} // Mesh
} // CF

#endif // CF_Mesh_SF_SFLineLagrangeP1
