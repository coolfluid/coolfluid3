// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_LagrangeP0_Triag_hpp
#define CF_Mesh_LagrangeP0_Triag_hpp

#include "Mesh/ShapeFunction.hpp"
#include "Mesh/GeoShape.hpp"
#include "Mesh/LagrangeP0/LibLagrangeP0.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeP0 {

/// @class Triag
/// @verbatim
/// Local connectivity:
///             .
///             | .
///             |   .
///             |     .
///             |   0   .
///             |         .
///             ------------
/// Reference domain: <0,1> x <0,1>
/// @endverbatim
class Mesh_LagrangeP0_API Triag  : public ShapeFunctionT<Triag> {
public:

  enum { dimensionality = 2 };
  enum { nb_nodes = 1 };
  enum { order = 1 };
  enum { shape = GeoShape::TRIAG };

//  static const Uint dimensionality = 2;
//  static const Uint nb_nodes = 1;
//  static const Uint order = 0;
//  static const GeoShape::Type shape = GeoShape::TRIAG;

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  /// Constructor
  Triag(const std::string& name = type_name());

  virtual ~Triag() {}

  /// Type name
  static std::string type_name() { return "Triag"; }

  /// Types for the matrices used
  typedef Eigen::Matrix<Real, dimensionality, 1> MappedCoordsT;
  typedef Eigen::Matrix<Real, 1, nb_nodes> ValueT;
  typedef Eigen::Matrix<Real, dimensionality, nb_nodes> GradientT;
  typedef Eigen::Matrix<Real, nb_nodes, dimensionality> MappedNodesT;

  static ValueT value(const MappedCoordsT& mapped_coord)
  {
    ValueT result;
    compute_value(mapped_coord,result);
    return result;
  }

  static void compute_value(const MappedCoordsT& mapped_coord, ValueT& result);

  static GradientT gradient(const MappedCoordsT& mapped_coord)
  {
    GradientT result;
    compute_gradient(mapped_coord,result);
    return result;
  }

  static void compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result);

  static const RealMatrix& local_coordinates() { return m_local_coordinates; }

public:

  static RealMatrix m_local_coordinates;

};

} // LagrangeP0
} // Mesh
} // CF

#endif // CF_Mesh_LagrangeP0_Triag_hpp
