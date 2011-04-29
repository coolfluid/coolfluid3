// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SF_SFPointLagrangeP1_hpp
#define CF_Mesh_SF_SFPointLagrangeP1_hpp

#include "Mesh/ShapeFunction.hpp"
#include "Mesh/GeoShape.hpp"
#include "Mesh/SF/LibSF.hpp"

namespace CF {
namespace Mesh {
namespace SF {

class MESH_SF_API SFPointLagrangeP1  : public ShapeFunction {
public:

  static const Uint dimensionality = 0;
  static const Uint nb_nodes = 1;
  static const Uint order = 1;
  static const GeoShape::Type shape = GeoShape::POINT;

public:

  /// Constructor
  SFPointLagrangeP1(const std::string& name = type_name());

  /// Type name
  static std::string type_name() { return "SFPointLagrangeP1"; }

  /// Types for the matrices used
  typedef Eigen::Matrix<Real, 1, 1> MappedCoordsT;
  typedef Eigen::Matrix<Real, 1, nb_nodes> ValueT;
  typedef Eigen::Matrix<Real, 1, nb_nodes> GradientT;
  typedef Eigen::Matrix<Real, nb_nodes, 1> MappedNodesT;

  /// Compute the shape functions corresponding to the given
  /// mapped coordinates
  /// @param mapped_coord The mapped coordinates
  /// @param result Vector storing the result
  static void value(const MappedCoordsT& mapped_coord, ValueT& result);

  /// Compute the gradient with respect to mapped coordinates, i.e. parial derivatives are in terms of the
  /// mapped coordinates. The result needs to be multiplied with the inverse jacobian to get the result in real
  /// coordinates.
  /// @param mappedCoord The mapped coordinates where the gradient should be calculated
  /// @param result Storage for the resulting gradient matrix (dimensionality x nb_nodes)
  static void gradient(const MappedCoordsT& mapped_coord, GradientT& result);

  /// Coordinates in mapped space of the nodes defining the shape function (nb_nodes x dimensionality)
  static const MappedNodesT& mapped_sf_nodes() { return s_mapped_sf_nodes; }

private:
  
  static MappedNodesT s_mapped_sf_nodes;
  
};

} // SF
} // Mesh
} // CF

#endif // CF_Mesh_SF_SFPointLagrangeP1
