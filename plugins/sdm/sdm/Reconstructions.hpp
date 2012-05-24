// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_Reconstructions_hpp
#define cf3_sdm_Reconstructions_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/Reconstructions.hpp"
#include "sdm/ShapeFunction.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {

////////////////////////////////////////////////////////////////////////////////

struct ReconstructFromFluxPoint : mesh::ReconstructBase
{
  /// Build coefficients for reconstruction in a given coordinate
  template <typename vector_type>
  void build_coefficients(const Uint direction, const vector_type& coord, const Handle<sdm::ShapeFunction const>& sf)
  {
    m_direction = direction;
    m_coord = coord;
    m_N.resize(sf->nb_flx_pts());
    sf->compute_flux_value(direction,coord,m_N);
    // Save indexes of non-zero values to speed up reconstruction
    m_pts.clear();
    for (Uint pt=0; pt<m_N.size(); ++pt)
    {
      if (std::abs(m_N[pt])>math::Consts::eps())
      {
        m_pts.push_back(pt);
      }
    }
    cf3_assert(used_points().size()>0);
  }

  Uint direction() const
  {
    return m_direction;
  }

private:
  Uint m_direction;
};

////////////////////////////////////////////////////////////////////////////////

struct DerivativeReconstructFromFluxPoint : mesh::ReconstructBase
{
  /// Build coefficients for reconstruction in a given coordinate
  template <typename vector_type>
  void build_coefficients(const Uint derivative, const vector_type& coord, const Handle<sdm::ShapeFunction const>& sf)
  {
    m_derivative = derivative;
    m_coord = coord;
    RealVector tmp(sf->nb_flx_pts());
    sf->compute_flux_derivative(derivative,coord,tmp);
    m_N = tmp;
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

struct ReconstructToFluxPoints
{

  void build_coefficients(const Handle<mesh::ShapeFunction const>& from_sf, const Handle<sdm::ShapeFunction const>& sf)
  {
    cf3_assert(sf);
    m_reconstruct.resize(sf->nb_flx_pts());
    for (Uint flx_pt=0; flx_pt<sf->nb_flx_pts(); ++flx_pt)
    {
      m_reconstruct[flx_pt].build_coefficients(sf->flx_pts().row(flx_pt),from_sf);
    }
  }

  void build_coefficients(const Handle<sdm::ShapeFunction const>& sf)
  {
    m_reconstruct.resize(sf->nb_flx_pts());
    for (Uint flx_pt=0; flx_pt<sf->nb_flx_pts(); ++flx_pt)
    {
      m_reconstruct[flx_pt].build_coefficients(sf->flx_pts().row(flx_pt),sf);
    }
  }

  const mesh::ReconstructPoint& operator[](const Uint flx_pt) const
  {
    return m_reconstruct[flx_pt];
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
  std::vector<mesh::ReconstructPoint> m_reconstruct;
};

////////////////////////////////////////////////////////////////////////////////

struct GradientReconstructToFluxPoints
{
  void build_coefficients(Handle<sdm::ShapeFunction const> sf)
  {
    m_derivative_reconstruct_to_flx_pt.resize(sf->nb_nodes());
    for (Uint pt=0; pt<sf->nb_nodes(); ++pt)
    {
      m_derivative_reconstruct_to_flx_pt[pt].resize(sf->dimensionality());
      for (Uint d=0; d<sf->dimensionality(); ++d)
        m_derivative_reconstruct_to_flx_pt[pt][d].build_coefficients(d,sf->flx_pts().row(pt),sf);
    }
  }

  void build_coefficients(const Handle<mesh::ShapeFunction const>& from_sf,Handle<sdm::ShapeFunction const> to_sf)
  {
    m_derivative_reconstruct_to_flx_pt.resize(to_sf->nb_flx_pts());
    for (Uint pt=0; pt<to_sf->nb_flx_pts(); ++pt)
    {
      m_derivative_reconstruct_to_flx_pt[pt].resize(to_sf->dimensionality());
      for (Uint d=0; d<to_sf->dimensionality(); ++d)
      {
        m_derivative_reconstruct_to_flx_pt[pt][d].build_coefficients(d,to_sf->flx_pts().row(pt),from_sf);
      }
    }
  }


  /// Double Operator [pt][derivative](from,to)
  /// @return derivative reconstruction in a given point
  const std::vector<mesh::DerivativeReconstructPoint>& operator[](const Uint pt) const
  {
    return m_derivative_reconstruct_to_flx_pt[pt];
  }

private:
  std::vector< std::vector<mesh::DerivativeReconstructPoint> > m_derivative_reconstruct_to_flx_pt;
};

////////////////////////////////////////////////////////////////////////////////

struct GradientReconstructFromFluxPoints
{
  void build_coefficients(const Handle<sdm::ShapeFunction const>& from_sf,Handle<mesh::ShapeFunction const> to_sf = Handle<mesh::ShapeFunction const>())
  {
    if ( is_null(to_sf) )
      to_sf=from_sf;
    m_derivative_reconstruct_from_flx_pt.resize(to_sf->nb_nodes());
    for (Uint pt=0; pt<to_sf->nb_nodes(); ++pt)
    {
      m_derivative_reconstruct_from_flx_pt[pt].resize(to_sf->dimensionality());
      for (Uint d=0; d<to_sf->dimensionality(); ++d)
        m_derivative_reconstruct_from_flx_pt[pt][d].build_coefficients(d,to_sf->local_coordinates().row(pt),from_sf);
    }
  }


  /// Double Operator [pt][derivative](from,to)
  /// @return derivative reconstruction in a given point
  const std::vector<DerivativeReconstructFromFluxPoint>& operator[](const Uint pt) const
  {
    return m_derivative_reconstruct_from_flx_pt[pt];
  }

private:
  std::vector< std::vector<DerivativeReconstructFromFluxPoint> > m_derivative_reconstruct_from_flx_pt;
};

////////////////////////////////////////////////////////////////////////////////

struct ReconstructFromFluxPoints
{
  void build_coefficients(const Handle<sdm::ShapeFunction const>& from_sf,Handle<mesh::ShapeFunction const> to_sf = Handle<mesh::ShapeFunction const>())
  {
    if ( is_null(to_sf) )
      to_sf=from_sf;
    m_reconstruct_from_flx_pt.resize(to_sf->nb_nodes());
    for (Uint pt=0; pt<to_sf->nb_nodes(); ++pt)
    {
      m_reconstruct_from_flx_pt[pt].resize(to_sf->dimensionality());
      for (Uint d=0; d<to_sf->dimensionality(); ++d)
        m_reconstruct_from_flx_pt[pt][d].build_coefficients(d,to_sf->local_coordinates().row(pt),from_sf);
    }
  }

  template <typename matrix_type>
  void set_zero(matrix_type& m) const
  {
    for (Uint i=0; i<m.size(); ++i) {
      for (Uint j=0; j<m[i].size(); ++j) {
        m[i][j]=0.;
      }
    }
  }
  void set_zero(RealMatrix& m) const
  {
    m.setZero();
  }

  /// Reconstruct values from matrix with values in row-vectors to matrix with values in row-vectors
  template <typename matrix_type_from, typename matrix_type_to>
  void operator()(const Uint direction, const matrix_type_from& from, matrix_type_to& to) const
  {
    set_zero(to);
    add(direction,from,to);
  }

  /// Reconstruct values from matrix with values in row-vectors to matrix with values in row-vectors
  template <typename matrix_type_from>
  void operator()(const Uint direction, const matrix_type_from& from, RealMatrix& to) const
  {
    set_zero(to);
    add(direction,from,to);
  }

  /// Reconstruct values from matrix with values in row-vectors to matrix with values in row-vectors
  template <typename matrix_type_from, typename matrix_type_to>
  void add(const Uint direction, const matrix_type_from& from, matrix_type_to& to) const
  {
    cf3_assert(m_reconstruct_from_flx_pt.size()==to.size());
    for (Uint r=0; r<m_reconstruct_from_flx_pt.size(); ++r) {
      m_reconstruct_from_flx_pt[r][direction].add(from,to[r]);
    }
  }

  /// Reconstruct values from matrix with values in row-vectors to matrix with values in row-vectors
  template <typename matrix_type_from>
  void add(const Uint direction, const matrix_type_from& from, RealMatrix& to) const
  {
    cf3_assert(m_reconstruct_from_flx_pt.size()==to.rows());
    for (Uint r=0; r<m_reconstruct_from_flx_pt.size(); ++r) {
      m_reconstruct_from_flx_pt[r][direction].add(from,to.row(r));
    }
  }

  template <typename matrix_type_from>
  RealMatrix operator()(const Uint direction, const matrix_type_from& from) const
  {
    RealMatrix to(m_reconstruct_from_flx_pt.size(),mesh::ReconstructBase::nb_vars(from));
    operator()(direction,from,to);
    return to;
  }

private:
  std::vector< std::vector<ReconstructFromFluxPoint> > m_reconstruct_from_flx_pt;
};

////////////////////////////////////////////////////////////////////////////////

struct DivergenceReconstructFromFluxPoints
{
  void build_coefficients(const Handle<sdm::ShapeFunction const>& from_sf,Handle<mesh::ShapeFunction const> to_sf = Handle<mesh::ShapeFunction const>())
  {
    m_ndims = from_sf->dimensionality();
    if ( is_null(to_sf) )
      to_sf=from_sf;
    m_derivative_reconstruct_from_flx_pt.resize(to_sf->nb_nodes());
    for (Uint pt=0; pt<to_sf->nb_nodes(); ++pt)
    {
      m_derivative_reconstruct_from_flx_pt[pt].resize(to_sf->dimensionality());
      for (Uint d=0; d<to_sf->dimensionality(); ++d)
        m_derivative_reconstruct_from_flx_pt[pt][d].build_coefficients(d,to_sf->local_coordinates().row(pt),from_sf);
    }
  }

  template <typename matrix_type>
  void set_zero(matrix_type& m) const
  {
    for (Uint i=0; i<m.size(); ++i) {
      for (Uint j=0; j<m[i].size(); ++j) {
        m[i][j]=0.;
      }
    }
  }
  void set_zero(RealMatrix& m) const
  {
    m.setZero();
  }

  /// Reconstruct values from matrix with values in row-vectors to matrix with values in row-vectors
  template <typename matrix_type_from, typename matrix_type_to>
  void operator()(const matrix_type_from& from, matrix_type_to& to) const
  {
    cf3_assert(m_derivative_reconstruct_from_flx_pt.size()==to.size());
    set_zero(to);
    for (Uint r=0; r<m_derivative_reconstruct_from_flx_pt.size(); ++r) {
      for (Uint d=0; d<ndims(); ++d) {
         m_derivative_reconstruct_from_flx_pt[r][d].add(from,to[r]);
      }
    }
  }

  /// Reconstruct values from matrix with values in row-vectors to matrix with values in row-vectors
  template <typename matrix_type_from>
  void operator()(const matrix_type_from& from, RealMatrix& to) const
  {
    cf3_assert(m_derivative_reconstruct_from_flx_pt.size()==to.rows());
    set_zero(to);
    for (Uint r=0; r<m_derivative_reconstruct_from_flx_pt.size(); ++r) {
      for (Uint d=0; d<ndims(); ++d) {
        m_derivative_reconstruct_from_flx_pt[r][d].add(from,to.row(r));
      }
    }
  }

  template <typename matrix_type_from>
  RealMatrix operator()(const matrix_type_from& from) const
  {
    RealMatrix to(m_derivative_reconstruct_from_flx_pt.size(),mesh::ReconstructBase::nb_vars(from));
    operator()(from,to);
    return to;
  }

  Uint ndims() const { return m_ndims; }

private:
  Uint m_ndims;
  std::vector< std::vector<DerivativeReconstructFromFluxPoint> > m_derivative_reconstruct_from_flx_pt;
};

////////////////////////////////////////////////////////////////////////////////

struct DivergenceFromFluxPointsToFluxPoints
{
  void build_coefficients(const Handle<sdm::ShapeFunction const>& sf)
  {
    m_ndims = sf->dimensionality();
    m_derivative_reconstruct_from_flx_pt.resize(sf->nb_flx_pts());
    for (Uint pt=0; pt<sf->nb_flx_pts(); ++pt)
    {
      m_derivative_reconstruct_from_flx_pt[pt].resize(sf->dimensionality());
      for (Uint d=0; d<m_ndims; ++d)
        m_derivative_reconstruct_from_flx_pt[pt][d].build_coefficients(d,sf->flx_pts().row(pt),sf);
    }
  }

  template <typename matrix_type>
  void set_zero(matrix_type& m) const
  {
    for (Uint i=0; i<m.size(); ++i) {
      for (Uint j=0; j<m[i].size(); ++j) {
        m[i][j]=0.;
      }
    }
  }
  void set_zero(RealMatrix& m) const
  {
    m.setZero();
  }

  /// Reconstruct values from matrix with values in row-vectors to matrix with values in row-vectors
  template <typename matrix_type_from, typename matrix_type_to>
  void operator()(const matrix_type_from& from, matrix_type_to& to) const
  {
    cf3_assert(m_derivative_reconstruct_from_flx_pt.size()==to.size());
    set_zero(to);
    for (Uint r=0; r<m_derivative_reconstruct_from_flx_pt.size(); ++r) {
      for (Uint d=0; d<ndims(); ++d) {
         m_derivative_reconstruct_from_flx_pt[r][d].add(from,to[r]);
      }
    }
  }

  /// Reconstruct values from matrix with values in row-vectors to matrix with values in row-vectors
  template <typename matrix_type_from>
  void operator()(const matrix_type_from& from, RealMatrix& to) const
  {
    cf3_assert(m_derivative_reconstruct_from_flx_pt.size()==to.rows());
    set_zero(to);
    for (Uint r=0; r<m_derivative_reconstruct_from_flx_pt.size(); ++r) {
      for (Uint d=0; d<ndims(); ++d) {
        m_derivative_reconstruct_from_flx_pt[r][d].add(from,to.row(r));
      }
    }
  }

  template <typename matrix_type_from>
  RealMatrix operator()(const matrix_type_from& from) const
  {
    RealMatrix to(m_derivative_reconstruct_from_flx_pt.size(),mesh::ReconstructBase::nb_vars(from));
    operator()(from,to);
    return to;
  }

  Uint ndims() const { return m_ndims; }

private:
  Uint m_ndims;
  std::vector< std::vector<DerivativeReconstructFromFluxPoint> > m_derivative_reconstruct_from_flx_pt;
};

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_Reconstructions_hpp
