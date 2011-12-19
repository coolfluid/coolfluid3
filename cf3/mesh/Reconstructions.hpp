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

#define ReconstructBase_Operation_Count
#ifdef ReconstructBase_Operation_Count
#define increase_elementary_operations ++ReconstructBase::elementary_operations
#else
#define increase_elementary_operations
#endif
struct ReconstructBase
{
  template <typename matrix_type>
  static Uint nb_vars(const matrix_type& m) { return m[0].size(); }
  static Uint nb_vars(const RealMatrix& m) { return m.cols(); }
  static Uint nb_vars(const boost::multi_array<Real, 2>& m) { return m.shape()[1]; }
  static Uint nb_vars(const boost::detail::multi_array::multi_array_view<Real, 2>& m) { return m.shape()[1]; }
  template <typename matrix_type>
  static const Real& access(const matrix_type& m, Uint i, Uint j) { return m[i][j]; }
  static const Real& access(const RealMatrix& m, Uint i, Uint j) { return m(i,j); }

  /// Reconstruct values from matrix with values in row-vectors to vector
  /// @param [in]  from  matrix with values in row-vectors
  /// @param [out] to    vector
  /// @note to is marked as const, but constness is casted away inside,
  ///       according to Eigen documentation http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
  template <typename matrix_type, typename vector_type>
  void operator()(const matrix_type& from, const vector_type& to) const
  {
    equal(from,to);
  }

  template <typename matrix_type, typename vector_type>
  void equal(const matrix_type& from, const vector_type& to) const
  {
    cf3_assert(m_pts.size()>0);
    set_zero(to);
    add(from,to);
  }

  template <typename matrix_type, typename vector_type>
  void add(const matrix_type& from, const vector_type& to) const
  {
    cf3_assert(m_pts.size()>0);
    boost_foreach(const Uint pt, m_pts)
      contribute_plus(from,to,pt);
  }

  template <typename matrix_type, typename vector_type>
  void subtract(const matrix_type& from, const vector_type& to) const
  {
    cf3_assert(m_pts.size()>0);
    boost_foreach(const Uint pt, m_pts)
      contribute_minus(from,to,pt);
  }


  /// @note vec is marked as const, but constness is casted away inside,
  ///       according to Eigen documentation http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
  template <typename vector_type>
  static void set_zero(const vector_type& vec)
  {
    //increase_elementary_operations;
    for (Uint var=0; var<vec.size(); ++var)
      const_cast<vector_type&>(vec)[var] = 0;
  }

  /// @note to is marked as const, but constness is casted away inside,
  ///       according to Eigen documentation http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
  template <typename matrix_type, typename vector_type>
  void contribute_plus(const matrix_type& from, const vector_type& to,const Uint pt) const
  {
    increase_elementary_operations;
    for (Uint var=0; var<nb_vars(from); ++var)
      const_cast<vector_type&>(to)[var] += m_N[pt] * access(from,pt,var);
  }

  /// @note to is marked as const, but constness is casted away inside,
  ///       according to Eigen documentation http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
  template <typename matrix_type, typename vector_type>
  void contribute_minus(const matrix_type& from, const vector_type& to,const Uint pt) const
  {
    increase_elementary_operations;
    for (Uint var=0; var<nb_vars(from); ++var)
      const_cast<vector_type&>(to)[var] -= m_N[pt] * access(from,pt,var);
  }

  const Real& coeff(const Uint pt) const
  {
    return m_N[pt];
  }

  const std::vector<Uint>& used_points() const
  {
    return m_pts;
  }

  static Uint elementary_operations;

protected:
  Handle<mesh::ShapeFunction const> m_sf;
  RealVector m_coord;
  RealRowVector m_N;
  std::vector<Uint> m_pts;
};
Uint ReconstructBase::elementary_operations=0u;

////////////////////////////////////////////////////////////////////////////////

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

struct DerivativeReconstructPoint : ReconstructBase
{
  /// Build coefficients for reconstruction in a given coordinate
  template <typename vector_type>
  void build_coefficients(const Uint derivative, const vector_type& coord, const Handle<mesh::ShapeFunction const>& sf)
  {
    m_derivative = derivative;
    m_coord = coord;
    m_N = sf->gradient(coord).col(derivative);

    // Save indexes of non-zero values to speed up reconstruction
    m_pts.clear();
    for (Uint pt=0; pt<m_N.size(); ++pt)
    {
      if (std::abs(m_N[pt])>math::Consts::eps())
      {
        m_pts.push_back(pt);
      }
    }
  }

  Uint derivative() const
  {
    return m_derivative;
  }

private:
  Uint m_derivative;
};

////////////////////////////////////////////////////////////////////////////////

struct Reconstruct
{
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

  const ReconstructPoint& operator[](const Uint pt) const
  {
    return m_reconstruct[pt];
  }

  /// Reconstruct values from matrix with values in row-vectors to matrix with values in row-vectors
  template <typename matrix_type_from, typename matrix_type_to>
  void operator()(const matrix_type_from& from, matrix_type_to& to) const
  {
    cf3_assert(m_reconstruct.size()==to.size());
    for (Uint r=0; r<m_reconstruct.size(); ++r)
      m_reconstruct[r](from,to[r]);
  }

  /// Reconstruct values from matrix with values in row-vectors to matrix with values in row-vectors
  template <typename matrix_type_from>
  void operator()(const matrix_type_from& from, RealMatrix& to) const
  {
    cf3_assert(m_reconstruct.size()==to.rows());
    for (Uint r=0; r<m_reconstruct.size(); ++r)
      m_reconstruct[r](from,to.row(r));
  }

private:
  Handle<mesh::ShapeFunction const> m_from_sf;
  Handle<mesh::ShapeFunction const> m_to_sf;
  std::vector<ReconstructPoint> m_reconstruct;
};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Reconstructions_hpp
