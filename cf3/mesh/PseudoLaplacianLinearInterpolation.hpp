// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_PseudoLaplacianLinearInterpolation_hpp
#define cf3_mesh_PseudoLaplacianLinearInterpolation_hpp

#include "mesh/InterpolationFunction.hpp"

namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////

/// @brief Pseudo-Laplacian Weighted Linear Interpolation function
///
/// This function computes interpolation weights based on distance of surrounding points to
/// the point of interest.
///
/// @author Willem Deconinck
class Mesh_API PseudoLaplacianLinearInterpolation : public InterpolationFunction
{
public:
  /// constructor
  PseudoLaplacianLinearInterpolation( const std::string& name ) : InterpolationFunction(name) {}

  /// Gets the Class name
  static std::string type_name() { return "PseudoLaplacianLinearInterpolation"; }


  virtual void compute_interpolation_weights(const RealVector& coordinate, const std::vector<SpaceElem>& stencil,
                                             std::vector<Uint>& source_field_points, std::vector<Real>& source_field_weights);


  /// @brief Pseudo-Laplacian weighted linear interpolation algorithm
  ///
  /// This is the core algorithm valid for 1D, 2D, or 3D.
  /// @param target_point  [in] The coordinate of the target point for interpolation
  /// @param source_points [in] The coordinates of the points used for interpolation
  /// @param weights [out]  The weights corresponding for each source_point.  Q_t = sum( weight_i * Q_i )
  static void pseudo_laplacian_weighted_linear_interpolation(const RealVector& t_point, const std::vector<RealVector>& s_points, std::vector<Real>& weights);

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_neu_PseudoLaplacianLinearInterpolation_hpp
