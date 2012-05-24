// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Reconstructions_hpp
#define cf3_mesh_Reconstructions_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/BoostArray.hpp"
#include "math/MatrixTypes.hpp"
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
struct ReconstructBase
{
  /// Reconstruct values from matrix with values in row-vectors to vector.
  /// The location of the vector must have been previously setup using
  /// build_coefficients()
  /// @param [in]  from  matrix with values in row-vectors
  /// @param [out] to    vector
  /// @note to is marked as const, but constness is casted away inside,
  ///       according to Eigen documentation http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
  template <typename matrix_type, typename vector_type>
  void operator()(const matrix_type& from, const vector_type& to) const
  {
    equal(from,to);
  }

  /// Reconstruct values from matrix with values in row-vectors to a vector.
  /// The location of the vector must have been previously setup using
  /// build_coefficients()
  /// @param [in]  from  matrix with values in row-vectors
  /// @param [out] to    vector
  /// @note to is marked as const, but constness is casted away inside,
  ///       according to Eigen documentation http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
  template <typename matrix_type, typename vector_type>
  void equal(const matrix_type& from, const vector_type& to) const
  {
//    cf3_assert(used_points().size()>0);
    set_zero(to);
    add(from,to);
  }

  /// Set the given vector to zero
  /// @note vec is marked as const, but constness is casted away inside,
  ///       according to Eigen documentation http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
  template <typename vector_type>
  static void set_zero(const vector_type& vec)
  {
    for (Uint var=0; var<vec.size(); ++var)
      const_cast<vector_type&>(vec)[var] = 0;
  }

  /// Reconstruct values from matrix with values in row-vectors to a vector.
  /// The location of the vector must have been previously setup using
  /// build_coefficients()
  /// The vector is not initialized to zero, hence the reconstructed values
  /// will be summed on top of the existing vector values
  /// @param [in]  from  matrix with values in row-vectors
  /// @param [out] to    vector
  /// @note to is marked as const, but constness is casted away inside,
  ///       according to Eigen documentation http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
  template <typename matrix_type, typename vector_type>
  void add(const matrix_type& from, const vector_type& to) const
  {
//    cf3_assert(used_points().size()>0);
    boost_foreach(const Uint pt, m_pts)
      contribute_plus(from,to,pt);
  }

  /// Reconstruct values from matrix with values in row-vectors to a vector.
  /// The location of the vector must have been previously setup using
  /// build_coefficients()
  /// The vector is not initialized to zero, hence the reconstructed values
  /// will be subtracted from the existing vector values
  /// @param [in]  from  matrix with values in row-vectors
  /// @param [out] to    vector
  /// @note to is marked as const, but constness is casted away inside,
  ///       according to Eigen documentation http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
  template <typename matrix_type, typename vector_type>
  void subtract(const matrix_type& from, const vector_type& to) const
  {
//    cf3_assert(used_points().size()>0);
    boost_foreach(const Uint pt, m_pts)
      contribute_minus(from,to,pt);
  }


  /// Add to a vector "to", the contribution of a given point "pt"
  /// coming from a matrix "from"
  /// @param [in]  from  matrix with values in row-vectors
  /// @param [in]  pt    point of which the contribution is added
  /// @param [out] to    vector
  /// @note to is marked as const, but constness is casted away inside,
  ///       according to Eigen documentation http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
  template <typename matrix_type, typename vector_type>
  void contribute_plus(const matrix_type& from, const vector_type& to,const Uint pt) const
  {
    for (Uint var=0; var<nb_vars(from); ++var)
      const_cast<vector_type&>(to)[var] += m_N[pt] * access(from,pt,var);
  }

  /// Subtract from a vector "to", the contribution of a given point "pt"
  /// coming from a matrix "from"
  /// @param [in]  from  matrix with values in row-vectors
  /// @param [in]  pt    point of which the contribution is added
  /// @param [out] to    vector
  /// @note to is marked as const, but constness is casted away inside,
  ///       according to Eigen documentation http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
  template <typename matrix_type, typename vector_type>
  void contribute_minus(const matrix_type& from, const vector_type& to,const Uint pt) const
  {
    for (Uint var=0; var<nb_vars(from); ++var)
      const_cast<vector_type&>(to)[var] -= m_N[pt] * access(from,pt,var);
  }

  /// Get the reconstruction coefficient of a given point
  const Real& coeff(const Uint pt) const
  {
    return m_N[pt];
  }

  /// Get a vector of points of which the reconstruction coefficient is not zero
  const std::vector<Uint>& used_points() const
  {
    return m_pts;
  }

protected: // functions

  // Adaptor functions to support both RealMatrix, multi_array, multi_array_view
  // as function arguments in this class
  template <typename matrix_type>
  static Uint nb_vars(const matrix_type& m) { return m[0].size(); }
  static Uint nb_vars(const RealMatrix& m) { return m.cols(); }
  static Uint nb_vars(const boost::multi_array<Real, 2>& m) { return m.shape()[1]; }
  static Uint nb_vars(const boost::detail::multi_array::multi_array_view<Real, 2>& m) { return m.shape()[1]; }
  template <typename matrix_type>
  static const Real& access(const matrix_type& m, Uint i, Uint j) { return m[i][j]; }
  static const Real& access(const RealMatrix& m, Uint i, Uint j) { return m(i,j); }

protected:
  Handle<mesh::ShapeFunction const> m_sf;
  RealVector m_coord;
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
///   reconstruct.build_coefficients(coord,shape_function);
///   reconstruct( matrix_of_node_values , vector_of_values_in_coord );
/// @author Willem Deconinck
struct ReconstructPoint : ReconstructBase
{
  /// Build coefficients for reconstruction in a given coordinate
  template <typename vector_type>
  void build_coefficients(const vector_type& coord, const Handle<mesh::ShapeFunction const>& sf)
  {
    m_coord = coord;
    m_pts.clear();
    m_N.resize(sf->nb_nodes());
    sf->compute_value(coord,m_N);

    // Save indexes of non-zero values to speed up reconstruction
    for (Uint pt=0; pt<m_N.size(); ++pt)
    {
      if (std::abs(m_N[pt])>math::Consts::eps())
      {
        m_pts.push_back(pt);
      }
    }
  }
};

////////////////////////////////////////////////////////////////////////////////

/// Reconstruction helper for the derivative in a given direction in a given coordinate.
/// Use:
///   DerivativeReconstructPoint reconstruct_derivative;
///   reconstruct_derivative.build_coefficients(orientation,coord,shape_function);
///   reconstruct_derivative( matrix_of_node_values , vector_of_derivatives_in_coord );
/// @author Willem Deconinck
struct DerivativeReconstructPoint : ReconstructBase
{
  /// Build coefficients for reconstruction in a given coordinate
  template <typename vector_type>
  void build_coefficients(const Uint derivative, const vector_type& coord, const Handle<mesh::ShapeFunction const>& sf)
  {
    m_derivative = derivative;
    m_coord = coord;
    RealMatrix grad_sf(sf->dimensionality(),sf->nb_nodes());
    sf->compute_gradient(coord,grad_sf);
    m_N = grad_sf.row(derivative);

//    std::cout << "grad_sf = \n" << sf->gradient(coord) << std::endl;
    // Save indexes of non-zero values to speed up reconstruction
    m_pts.clear();
    for (Uint pt=0; pt<m_N.size(); ++pt)
    {
      if (std::abs(m_N[pt])>math::Consts::eps())
      {
        m_pts.push_back(pt);
      }
    }
//    cf3_assert(used_points().size()>0);
  }

  Uint derivative() const
  {
    return m_derivative;
  }

private:
  Uint m_derivative;
};

////////////////////////////////////////////////////////////////////////////////

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
    cf3_assert(m_reconstruct.size()==to.size());
    for (Uint pt=0; pt<m_reconstruct.size(); ++pt)
      m_reconstruct[pt](from,to[pt]);
  }

  /// Reconstruct values from matrix with values in row-vectors to matrix with values in row-vectors
  template <typename matrix_type_from>
  void operator()(const matrix_type_from& from, RealMatrix& to) const
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

////////////////////////////////////////////////////////////////////////////////


} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Reconstructions_hpp
