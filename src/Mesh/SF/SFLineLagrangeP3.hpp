// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SF_SFLineLagrangeP3_hpp
#define CF_Mesh_SF_SFLineLagrangeP3_hpp

#include "Mesh/ShapeFunction.hpp"
#include "Mesh/GeoShape.hpp"
#include "Mesh/SF/LibSF.hpp"

namespace CF {
namespace Mesh {
namespace SF {

class MESH_SF_API SFLineLagrangeP3  : public ShapeFunction {
public:

  static const Uint dimensionality = 1;
  static const Uint nb_nodes = 4;
  static const Uint order = 3;
  static const GeoShape::Type shape = GeoShape::LINE;

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  /// Constructor
  SFLineLagrangeP3(const std::string& name = type_name());

  /// Type name
  static std::string type_name() { return "SFLineLagrangeP3"; }

  /// Types for the matrices used
  typedef Eigen::Matrix<Real, dimensionality, 1> MappedCoordsT;
  typedef Eigen::Matrix<Real, 1, nb_nodes> ValueT;
  typedef Eigen::Matrix<Real, dimensionality, nb_nodes> GradientT;
  typedef Eigen::Matrix<Real, nb_nodes, dimensionality> MappedNodesT;

  /// Compute the shape functions corresponding to the given
  /// mapped coordinates
  /// @param mapped_coord The mapped coordinates
  /// @param result Vector storing the result
  static void compute_value(const MappedCoordsT& mapped_coord, ValueT& result);

  /// Compute the gradient with respect to mapped coordinates, i.e. parial derivatives are in terms of the
  /// mapped coordinates. The result needs to be multiplied with the inverse jacobian to get the result in real
  /// coordinates.
  /// @param mapped_coord The mapped coordinates where the gradient should be calculated (dimensionality x nb_nodes)
  /// @param result Storage for the resulting gradient matrix
  static void compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result);

  /// Coordinates in mapped space of the nodes defining the shape function (nb_nodes x dimensionality)
  static const MappedNodesT& mapped_sf_nodes() { return s_mapped_sf_nodes; }

  virtual RealRowVector value(const RealVector& local_coord) const
  {
    ValueT result;
    compute_value(local_coord,result);
    return result;
  }

  virtual RealMatrix gradient(const RealVector& local_coord) const
  {
    GradientT result;
    compute_gradient(local_coord,result);
    return result;
  }

  virtual const RealMatrix& local_coordinates() const
  {
    return s_mapped_sf_nodes;
  }

private:

  static RealMatrix s_mapped_sf_nodes;

};

} // SF
} // Mesh
} // CF

#endif // CF_Mesh_SF_SFLineLagrangeP3
