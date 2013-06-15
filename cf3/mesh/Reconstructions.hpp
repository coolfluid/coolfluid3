// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Reconstructions_hpp
#define cf3_mesh_Reconstructions_hpp

////////////////////////////////////////////////////////////////////////////////

#include "math/MatrixTypes.hpp"
#include "math/Consts.hpp"
#include "mesh/ShapeFunction.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////

/// Base class to help reconstruction of element values to any given coordinate
/// value_in_coord = sum ( N(i) * value_in_node(i) )
/// Derived classes have to implement the actual computation of the
/// values "N(i)". Optimization is allowed by storing an internal vector
/// m_pts, holding the indices of only the non-zero N(i)'s.
/// @author Willem Deconinck
struct ReconstructPoint
{
  /// Get the reconstruction coefficient of a given point
  const Real& coeff(const Uint pt) const
  {
    cf3_assert( pt<m_N.size() );
    return m_N[pt];
  }

  /// Get a vector of points of which the reconstruction coefficient is not zero
  const std::vector<Uint>& used_points() const
  {
    return m_pts;
  }

  void construct_used_points()
  {
    m_pts.clear(); m_pts.reserve(m_N.size());
    // Save indexes of non-zero values to speed up reconstruction
    for (Uint pt=0; pt<m_N.size(); ++pt)
    {
      if (std::abs(m_N[pt])>math::Consts::eps())
      {
        m_pts.push_back(pt);
      }
    }
  }

  RealRowVector m_N;
  std::vector<Uint> m_pts;
};

////////////////////////////////////////////////////////////////////////////////

/// Reconstruction helper, implementing a function to calculate
/// values N(i) for a given coordinate, and optimizing by storing
/// indices i of non-zero N(i)'s.
///
/// Use:
///   ReconstructPoint reconstruct;
///   InterpolateInPoint::build_coefficients(reconstruct,coord,shape_function);
/// @author Willem Deconinck
struct InterpolateInPoint
{
  /// Build coefficients for reconstruction in a given coordinate
  template <typename vector_type>
  static void build_coefficients( ReconstructPoint& reconstruction,
                                  const vector_type& local_coord,
                                  const Handle<mesh::ShapeFunction const>& sf )
  {
    reconstruction.m_N.resize(sf->nb_nodes());
    sf->compute_value(local_coord,reconstruction.m_N);
    reconstruction.construct_used_points();
    cf3_always_assert(reconstruction.used_points().size());
  }
};

////////////////////////////////////////////////////////////////////////////////

/// Reconstruction helper for the derivative in a given direction in a given coordinate.
/// Use:
///   DerivativeReconstructPoint reconstruct_derivative;
///   reconstruct_derivative.build_coefficients(orientation,coord,shape_function);
///   reconstruct_derivative( matrix_of_node_values , vector_of_derivatives_in_coord );
/// @author Willem Deconinck
struct DerivativeInPoint
{
  /// Build coefficients for reconstruction in a given coordinate
  template <typename vector_type>
  static void build_coefficients( ReconstructPoint& reconstruction,
                                  const Uint derivative_to,
                                  const vector_type& local_coord,
                                  const Handle<mesh::ShapeFunction const>& sf )
  {
    RealMatrix grad_sf(sf->dimensionality(),sf->nb_nodes());
    sf->compute_gradient(local_coord,grad_sf);
    reconstruction.m_N = grad_sf.row(derivative_to);
    reconstruction.construct_used_points();
  }
};

////////////////////////////////////////////////////////////////////////////////

#if 0
/// Reconstruction helper that reconstructs values matching one shape function
/// to values matching another shape function.
/// This is good to interpolate between different spaces.
/// Use:
///   Reconstruct reconstruct;
///   reconstruct.build_coefficients( from_shape_function, to_shape_function);
///   reconstruct( matrix_of_from_values , matrix_of_to_values );
/// @author Willem Deconinck
struct Reconstruct
{
  /// Build the coefficients for the reconstruction for every point of to_sf
  void build_coefficients(const Handle<mesh::ShapeFunction const>& from_sf,
                          const Handle<mesh::ShapeFunction const>& to_sf)
  {
    m_from_sf=from_sf;
    m_to_sf=to_sf;
    m_reconstruct.resize(to_sf->nb_nodes());
    for (Uint pt=0; pt<to_sf->nb_nodes(); ++pt)
    {
      m_reconstruct[pt].build_coefficients(to_sf->local_coordinates().row(pt),from_sf);
    }
  }

  /// Access to individual reconstructor for one point
  const ReconstructPoint& operator[](const Uint pt) const
  {
    return m_reconstruct[pt];
  }

  /// Reconstruct values from matrix with values in row-vectors to matrix with values in row-vectors
  template <typename matrix_type_from, typename matrix_type_to>
  void operator()(const matrix_type_from& from, matrix_type_to& to) const
  {
    cf3_assert(m_reconstruct.size()==to.rows());
    for (Uint pt=0; pt<m_reconstruct.size(); ++pt)
      m_reconstruct[pt](from,to.row(pt));
  }

private:
  Handle<mesh::ShapeFunction const> m_from_sf;
  Handle<mesh::ShapeFunction const> m_to_sf;
  std::vector<ReconstructPoint> m_reconstruct;
};

////////////////////////////////////////////////////////////////////////////////

/// @author Willem Deconinck
struct GradientReconstruct : ReconstructBase
{
  /// Build the coefficients for the reconstruction for every point of to_sf
  void build_coefficients(const Handle<mesh::ShapeFunction const>& from_sf,
                          const Handle<mesh::ShapeFunction const>& to_sf)
  {
    m_from_sf=from_sf;
    m_to_sf=to_sf;
    m_derivativereconstruct.resize(from_sf->dimensionality());
    for (Uint d=0; d<m_derivativereconstruct.size(); ++d)
    {
      m_derivativereconstruct[d].resize(to_sf->nb_nodes());
      for (Uint pt=0; pt<to_sf->nb_nodes(); ++pt)
      {
        m_derivativereconstruct[d][pt].build_coefficients(d,to_sf->local_coordinates().row(pt),from_sf);
      }
    }
  }

  /// Access to individual reconstructor for one derivative
  const std::vector<DerivativeReconstructPoint>& operator[](const Uint d) const
  {
    cf3_assert(d<m_derivativereconstruct.size());
    return m_derivativereconstruct[d];
  }

private:
  Handle<mesh::ShapeFunction const> m_from_sf;
  Handle<mesh::ShapeFunction const> m_to_sf;
  std::vector< std::vector<DerivativeReconstructPoint> > m_derivativereconstruct;
  Uint m_dim;
};
#endif
////////////////////////////////////////////////////////////////////////////////


} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Reconstructions_hpp
