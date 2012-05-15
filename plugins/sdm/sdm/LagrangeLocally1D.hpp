// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_ShapeFunctionT_hpp
#define cf3_sdm_ShapeFunctionT_hpp

#include "common/BoostArray.hpp"
#include "common/StringConversion.hpp"

#include "math/MatrixTypes.hpp"
#include "math/Consts.hpp"

#include "sdm/ShapeFunction.hpp"
#include "sdm/LibSDM.hpp"

namespace cf3 {
namespace sdm {

////////////////////////////////////////////////////////////////////////////////

struct sdm_API Lagrange
{
  static Real coeff(const Real& ksi, const RealVector& pts, const Uint idx)
  {
    // declarations for efficiency
    Uint k;
    const Uint nb_pts(pts.size());

    Real prod=1;
    for(k=0; k<nb_pts;++k)
    {
      if (k!=idx)
        prod*= (ksi-pts[k])/(pts[idx]-pts[k]);
    }
    return prod;
  }

  static Real deriv_coeff(const Real& ksi, const RealVector& pts, const Uint idx)
  {
    // declarations for efficiency
    Uint t;
    Uint k;
    Real term;
    const Uint nb_pts(pts.size());

     Real deriv_coeff = 0.;
     for (t = 0; t < nb_pts; ++t)
     {
       if (t != idx)
       {
         term = 1./(pts[idx]-pts[t]);
         for (k = 0; k < nb_pts; ++k)
         {
           if (k != idx && k != t)
           {
             term *= (ksi-pts[k])/(pts[idx]-pts[k]);
           }
         }
         deriv_coeff += term;
       }
     }
     return deriv_coeff;
  }
};

////////////////////////////////////////////////////////////////////////////////

struct sdm_API Locally_1d
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW  ///< storing fixed-sized Eigen structures

  Locally_1d(const Uint p)
  {
    RealVector sol_pts(p+1);
    RealVector flx_pts(p+2);
    switch (p)
    {
    case 0:
      flx_pts << -1,1;
      break;
    case 1:
      flx_pts << -1,0,1;
      break;
    case 2:
      flx_pts << -1,-1/sqrt(3.),1/sqrt(3.),1;
      break;
    case 3:
      flx_pts << -1,-sqrt(3./5.),0,+sqrt(3./5.),1;
      break;
    case 4:
      flx_pts << -1, -sqrt((3.+2.*sqrt(6./5.))/7.), -sqrt((3.-2.*sqrt(6./5.))/7.), +sqrt((3.-2.*sqrt(6./5.))/7.), +sqrt((3.+2.*sqrt(6./5.))/7.), 1;
      break;
    case 5:
      flx_pts << -1, -sqrt(5.+2.*sqrt(10./7.))/3., -sqrt(5.-2.*sqrt(10./7.))/3., 0., +sqrt(5.-2.*sqrt(10./7.))/3., +sqrt(5.+2.*sqrt(10./7.))/3., +1;
      break;
    default:
      throw common::NotImplemented(FromHere(),"1D flux-point locations for P"+common::to_str(p)+" are not yet defined");
      break;
    }

    // Make solution points symmetric and collocated as much as possible with flux points
    Uint s = 0;
    for (Uint f = 0; f < sol_pts.size()/2; ++f, ++s)
      sol_pts[s] = flx_pts[f];
    if (sol_pts.size()%2 == 1)
    {
      sol_pts[s] = 0.5*(flx_pts[sol_pts.size()/2]+flx_pts[sol_pts.size()/2+1]);
      ++s;
    }
    for (Uint f = flx_pts.size()/2+1; f < flx_pts.size(); ++f, ++s)
      sol_pts[s] = flx_pts[f];

    setup(sol_pts,flx_pts);
  }

  Locally_1d(const RealVector& sol_pts_1d, const RealVector& flx_pts_1d)
  {
    setup(sol_pts_1d,flx_pts_1d);
  }

  void setup(const RealVector& sol_pts_1d, const RealVector& flx_pts_1d)
  {
    sol_pts = sol_pts_1d;
    flx_pts = flx_pts_1d;
    nb_sol_pts = sol_pts.size();
    nb_flx_pts = flx_pts.size();

    interpolate_sol_to_flx.resize(flx_pts.size(),sol_pts.size());
    for (Uint f=0; f<flx_pts.size(); ++f) {
      for (Uint s=0; s<sol_pts.size(); ++s) {
        interpolate_sol_to_flx(f,s) = Lagrange::coeff(flx_pts[f],sol_pts,s);
      }
    }

    interpolate_flx_to_sol.resize(sol_pts.size(),flx_pts.size());
    for (Uint s=0; s<sol_pts.size(); ++s) {
      for (Uint f=0; f<flx_pts.size(); ++f) {
        interpolate_flx_to_sol(s,f) = Lagrange::coeff(sol_pts[s],flx_pts,f);
      }
    }

    interpolate_grad_flx_to_sol.resize(sol_pts.size(),flx_pts.size());
    for (Uint s=0; s<sol_pts.size(); ++s) {
      for (Uint f=0; f<flx_pts.size(); ++f) {
        interpolate_grad_flx_to_sol(s,f) = Lagrange::deriv_coeff(sol_pts[s],flx_pts,f);
      }
    }
  }

  RealVector sol_pts;
  RealVector flx_pts;
  Uint nb_sol_pts;
  Uint nb_flx_pts;

  RealMatrix interpolate_sol_to_flx;
  RealMatrix interpolate_flx_to_sol;
  RealMatrix interpolate_grad_flx_to_sol;
};

////////////////////////////////////////////////////////////////////////////////

template<Uint P>
class sdm_API Point : public sdm::ShapeFunction
{
public: // typedefs

  typedef boost::shared_ptr<Point>       Ptr;
  typedef boost::shared_ptr<Point const> ConstPtr;

public: // functions

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW  ///< storing fixed-sized Eigen structures

  static std::string type_name() { return "Point<"+common::to_str(P)+">"; }

  Point(const std::string& name = type_name())
    : sdm::ShapeFunction(name),
      m_flx_pt_dirs(1,KSI),
      m_order(P),
      m_zero(0.),
      m_one(1.)
  {
    m_sol_pts.resize(1,3);
    m_sol_pts << 0, 0, 0;
    m_flx_pts.resize(1,3);
    m_flx_pts << 0, 0, 0;

    m_interp_grad_flx_to_sol_used_sol_pts.resize(1);
    m_interp_sol_to_flx_used_sol_pts.resize(1);
    m_interp_flx_to_sol_used_sol_pts.resize(1);
    m_face_flx_pts.resize(0);
    m_flx_pt_sign.resize(1,1.);

    m_interp_grad_flx_to_sol_used_sol_pts[0].push_back(0);
    m_interp_flx_to_sol_used_sol_pts[0].push_back(0);
    m_interp_sol_to_flx_used_sol_pts[0].push_back(0);

//    m_face_normals.resize(0);
  }

  virtual ~Point() {}

  virtual void compute_value(const RealVector& local_coordinate, RealRowVector& value) const
  {
    value[0] = 1.;
  }
  virtual void compute_gradient(const RealVector& local_coordinate, RealMatrix& gradient) const
  {
    gradient.setZero();
  }
  virtual void compute_flux_value(const Uint orientation, const RealVector& local_coordinate, RealRowVector& value) const
  {
    value[0] = 1.;
  }
  virtual void compute_flux_derivative(const Uint orientation, const RealVector& local_coordinate, RealVector& derivative) const
  {
    derivative.setZero();
  }
  virtual const std::vector<Uint>& interpolate_grad_flx_to_sol_used_sol_pts(const Uint flx_pt, const Uint direction) const
  {
    // direction doesn't play a role because every flx_pt is uniquely defined for 1 direction
    return m_interp_grad_flx_to_sol_used_sol_pts[flx_pt];
  }
  virtual const std::vector<Uint>& interpolate_sol_to_flx_used_sol_pts(const Uint flx_pt) const
  {
    // direction doesn't play a role because every flx_pt is uniquely defined for 1 direction,
    // and if it would play a role, then just more solution points would be added in more directions
    return m_interp_sol_to_flx_used_sol_pts[flx_pt];
  }
  virtual const std::vector<Uint>& interpolate_flx_to_sol_used_sol_pts(const Uint flx_pt, const Uint direction) const
  {
    // direction plays a role because interpolation in different directions can be of a different order
    return m_interp_flx_to_sol_used_sol_pts[flx_pt];
  }
  virtual const Real& interpolate_grad_flx_to_sol_coeff(const Uint flx_pt, const Uint direction, const Uint sol_pt) const
  {
    // direction plays a role because every sol_pt is used in multiple directions
    return m_zero;
  }
  virtual const Real& interpolate_sol_to_flx_coeff(const Uint flx_pt, const Uint sol_pt) const
  {
    return m_one;
  }
  virtual const Real& interpolate_flx_to_sol_coeff(const Uint flx_pt, const Uint direction, const Uint sol_pt) const
  {
    return m_one;
  }
  virtual mesh::GeoShape::Type shape() const { return mesh::GeoShape::POINT; }
  virtual Uint dimensionality() const { return DIM_3D; }
  virtual Uint nb_faces() const { return 0; }
  virtual Uint order() const { return m_order; }
  virtual Uint nb_sol_pts() const { return 1; }
  virtual Uint nb_flx_pts() const { return 1; }
  virtual const RealMatrix& sol_pts() const { return m_sol_pts; }
  virtual const RealMatrix& flx_pts() const { return m_flx_pts; }
  virtual const std::vector<Uint>& flx_pt_dirs(const Uint flx_pt) const { cf3_assert(flx_pt<nb_flx_pts()); return m_flx_pt_dirs; }
  virtual const RealMatrix& face_normals() const { return m_face_normals; }
  virtual const std::vector<Uint>& interior_flx_pts() const { return m_interior_flx_pts; }
  virtual const std::vector<Uint>& face_flx_pts(const Uint face_idx) const { cf3_assert(face_idx<nb_faces()); return m_face_flx_pts[face_idx]; }
  virtual const Real& flx_pt_sign(const Uint flx_pt, const Uint dir) const { return m_flx_pt_sign[flx_pt]; }

private: // data

  Real m_zero;
  Real m_one;

  Uint                                m_order;            ///< Order of the solution shape function
  RealMatrix                          m_flx_pts;          ///< Flux point coordinates
  RealMatrix                          m_sol_pts;          ///< Solution point coordinates
  std::vector<Uint>                   m_flx_pt_dirs;      ///< Per flux point, the directions this flux point contributes to
  std::vector< std::vector<Uint> >    m_interp_grad_flx_to_sol_used_sol_pts;  ///< Per flux point, the solution points used in the derivatives
  std::vector< std::vector<Uint> >    m_interp_sol_to_flx_used_sol_pts; ///< Per flux point, the solution points used to interpolate it
  std::vector< std::vector<Uint> >    m_interp_flx_to_sol_used_sol_pts; ///< Per flux point, the solution points used in the interpolation
  RealMatrix                          m_face_normals;     ///< Rows are normals to faces according to FaceNumbering
  std::vector<Uint>                   m_interior_flx_pts; ///< Flux points that lie inside the cell, not on the faces
  std::vector<std::vector<Uint> >     m_face_flx_pts;     ///< Flux points that on the cell faces
  std::vector<Real>                   m_flx_pt_sign;                          ///< Sign to be multiplied with computed flux in flx_pt in direction dir

};

////////////////////////////////////////////////////////////////////////////////

template<Uint P>
class sdm_API LineLagrange1D : public sdm::ShapeFunction
{
public: // typedefs

  typedef boost::shared_ptr<LineLagrange1D>       Ptr;
  typedef boost::shared_ptr<LineLagrange1D const> ConstPtr;

private: // typedefs

  enum FaceNumbering {KSI_NEG=0, KSI_POS=1};

public: // functions

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW  ///< storing fixed-sized Eigen structures

  static std::string type_name() { return "LineLagrange1D<"+common::to_str(P)+">"; }

  LineLagrange1D(const std::string& name = type_name())
    : sdm::ShapeFunction(name),
      m_local_1d( P ),
      m_flx_pt_dirs(1,KSI),
      m_order(m_local_1d.nb_sol_pts-1)
  {
    m_sol_pts.resize(m_local_1d.nb_sol_pts,1);
    for (Uint s=0; s<m_local_1d.nb_sol_pts; ++s)
      m_sol_pts(s,KSI)=m_local_1d.sol_pts[s];

    m_flx_pts.resize(m_local_1d.nb_flx_pts,1);
    for (Uint f=0; f<m_local_1d.nb_flx_pts; ++f)
      m_flx_pts(f,KSI)=m_local_1d.flx_pts[f];

    m_interp_grad_flx_to_sol_used_sol_pts.resize(m_local_1d.nb_flx_pts);
    m_interp_sol_to_flx_used_sol_pts.resize(m_local_1d.nb_flx_pts);
    m_interp_flx_to_sol_used_sol_pts.resize(m_local_1d.nb_flx_pts);
    m_face_flx_pts.resize(2);
    m_flx_pt_sign.resize(m_local_1d.nb_flx_pts,1.);

    for (Uint f=0; f<m_local_1d.nb_flx_pts; ++f)
    {
      for (Uint s=0; s<m_local_1d.nb_sol_pts; ++s)
      {

        if (std::abs( m_local_1d.interpolate_grad_flx_to_sol(s,f)) > 100*math::Consts::eps())
          m_interp_grad_flx_to_sol_used_sol_pts[f].push_back(s);

        if (std::abs( m_local_1d.interpolate_flx_to_sol(s,f)) > 100*math::Consts::eps())
          m_interp_flx_to_sol_used_sol_pts[f].push_back(s);

        if (std::abs( m_local_1d.interpolate_sol_to_flx(f,s)) > 100*math::Consts::eps())
          m_interp_sol_to_flx_used_sol_pts[f].push_back(s);
      }

      if (f==0)
      {
        m_face_flx_pts[KSI_NEG].push_back(f);
        m_flx_pt_sign[f]=-1.;
      }
      else if(f==m_local_1d.nb_flx_pts-1)
      {
        m_face_flx_pts[KSI_POS].push_back(f);
        m_flx_pt_sign[f]=+1.;
      }
      else
      {
        m_interior_flx_pts.push_back(f);
        m_flx_pt_sign[f]=+1.;
      }
    }

    m_face_normals.resize(nb_faces(),DIM_1D); m_face_normals.setZero();
    m_face_normals(KSI_NEG,KSI)=-1;
    m_face_normals(KSI_POS,KSI)=+1;

  }

  virtual ~LineLagrange1D() {}

  virtual void compute_value(const RealVector& local_coordinate, RealRowVector& value) const
  {
    cf3_assert(value.size()==m_local_1d.nb_sol_pts);
    for (Uint s=0; s<m_local_1d.nb_sol_pts; ++s) {
      value[s] = Lagrange::coeff(local_coordinate[KSI],m_local_1d.sol_pts,s);
    }
  }
  virtual void compute_gradient(const RealVector& local_coordinate, RealMatrix& gradient) const
  {
    cf3_assert(gradient.rows()==DIM_1D);
    cf3_assert(gradient.cols()==m_local_1d.nb_sol_pts);
    for (Uint s=0; s<m_local_1d.nb_sol_pts; ++s) {
      gradient(KSI,s) = Lagrange::deriv_coeff(local_coordinate[KSI],m_local_1d.sol_pts,s);
    }
  }
  virtual void compute_flux_value(const Uint orientation, const RealVector& local_coordinate, RealRowVector& value) const
  {
    cf3_assert(value.size()==nb_flx_pts());
    cf3_assert(orientation==KSI);
    value.setZero();
    for (Uint f_ksi=0; f_ksi<m_local_1d.nb_flx_pts; ++f_ksi)
      value[f_ksi] = Lagrange::coeff(local_coordinate[KSI],m_local_1d.flx_pts,f_ksi);
  }
  virtual void compute_flux_derivative(const Uint orientation, const RealVector& local_coordinate, RealVector& derivative) const
  {
    cf3_assert(derivative.size()==nb_flx_pts());
    cf3_assert(orientation==KSI);
    derivative.setZero();
    for (Uint f_ksi=0; f_ksi<m_local_1d.nb_flx_pts; ++f_ksi)
      derivative[f_ksi] = Lagrange::deriv_coeff(local_coordinate[KSI],m_local_1d.flx_pts,f_ksi);
  }
  virtual const std::vector<Uint>& interpolate_grad_flx_to_sol_used_sol_pts(const Uint flx_pt, const Uint direction) const
  {
    // direction doesn't play a role because every flx_pt is uniquely defined for 1 direction
    return m_interp_grad_flx_to_sol_used_sol_pts[flx_pt];
  }
  virtual const std::vector<Uint>& interpolate_sol_to_flx_used_sol_pts(const Uint flx_pt) const
  {
    // direction doesn't play a role because every flx_pt is uniquely defined for 1 direction,
    // and if it would play a role, then just more solution points would be added in more directions
    return m_interp_sol_to_flx_used_sol_pts[flx_pt];
  }
  virtual const std::vector<Uint>& interpolate_flx_to_sol_used_sol_pts(const Uint flx_pt, const Uint direction) const
  {
    // direction plays a role because interpolation in different directions can be of a different order
    return m_interp_flx_to_sol_used_sol_pts[flx_pt];
  }
  virtual const Real& interpolate_grad_flx_to_sol_coeff(const Uint flx_pt, const Uint direction, const Uint sol_pt) const
  {
    // direction plays a role because every sol_pt is used in multiple directions
    return m_local_1d.interpolate_grad_flx_to_sol(sol_pt,flx_pt);
  }
  virtual const Real& interpolate_sol_to_flx_coeff(const Uint flx_pt, const Uint sol_pt) const
  {
    return m_local_1d.interpolate_sol_to_flx(flx_pt,sol_pt);
  }
  virtual const Real& interpolate_flx_to_sol_coeff(const Uint flx_pt, const Uint direction, const Uint sol_pt) const
  {
    return m_local_1d.interpolate_flx_to_sol(sol_pt,flx_pt);
  }
  virtual mesh::GeoShape::Type shape() const { return mesh::GeoShape::LINE; }
  virtual Uint dimensionality() const { return DIM_1D; }
  virtual Uint nb_faces() const { return 2; }
  virtual Uint order() const { return m_order; }
  virtual Uint nb_sol_pts() const { return m_local_1d.nb_sol_pts; }
  virtual Uint nb_flx_pts() const { return m_local_1d.nb_flx_pts; }
  virtual const RealMatrix& sol_pts() const { return m_sol_pts; }
  virtual const RealMatrix& flx_pts() const { return m_flx_pts; }
  virtual const std::vector<Uint>& flx_pt_dirs(const Uint flx_pt) const { cf3_assert(flx_pt<nb_flx_pts()); return m_flx_pt_dirs; }
  virtual const RealMatrix& face_normals() const { return m_face_normals; }
  virtual const std::vector<Uint>& interior_flx_pts() const { return m_interior_flx_pts; }
  virtual const std::vector<Uint>& face_flx_pts(const Uint face_idx) const { cf3_assert(face_idx<nb_faces()); return m_face_flx_pts[face_idx]; }
  virtual const Real& flx_pt_sign(const Uint flx_pt, const Uint dir) const { return m_flx_pt_sign[flx_pt]; }

private: // data

  Locally_1d                          m_local_1d;         ///< holds 1D interpolation matrices
  Uint                                m_order;            ///< Order of the solution shape function
  RealMatrix                          m_flx_pts;          ///< Flux point coordinates
  RealMatrix                          m_sol_pts;          ///< Solution point coordinates
  std::vector<Uint>                   m_flx_pt_dirs;      ///< Per flux point, the directions this flux point contributes to
  std::vector< std::vector<Uint> >    m_interp_grad_flx_to_sol_used_sol_pts;  ///< Per flux point, the solution points used in the derivatives
  std::vector< std::vector<Uint> >    m_interp_sol_to_flx_used_sol_pts; ///< Per flux point, the solution points used to interpolate it
  std::vector< std::vector<Uint> >    m_interp_flx_to_sol_used_sol_pts; ///< Per flux point, the solution points used in the interpolation
  RealMatrix                          m_face_normals;     ///< Rows are normals to faces according to FaceNumbering
  std::vector<Uint>                   m_interior_flx_pts; ///< Flux points that lie inside the cell, not on the faces
  std::vector<std::vector<Uint> >     m_face_flx_pts;     ///< Flux points that on the cell faces
  std::vector<Real>                   m_flx_pt_sign;                          ///< Sign to be multiplied with computed flux in flx_pt in direction dir

};

////////////////////////////////////////////////////////////////////////////////

template<Uint P>
class sdm_API QuadLagrange1D : public sdm::ShapeFunction
{
public: // typedefs

  typedef boost::shared_ptr<QuadLagrange1D>       Ptr;
  typedef boost::shared_ptr<QuadLagrange1D const> ConstPtr;

private: // typedefs

  enum FaceNumbering {ETA_NEG=0, KSI_POS=1, ETA_POS=2, KSI_NEG=3};

public: // functions

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW  ///< storing fixed-sized Eigen structures

  static std::string type_name() { return "QuadLagrange1D<"+common::to_str(P)+">"; }

  QuadLagrange1D(const std::string& name = type_name())
    : sdm::ShapeFunction(name),
      m_local_1d(P)
  {
    m_order = m_local_1d.nb_sol_pts-1;
    m_nb_sol_pts = m_local_1d.nb_sol_pts*m_local_1d.nb_sol_pts;
    m_nb_flx_pts = m_local_1d.nb_sol_pts*m_local_1d.nb_flx_pts*DIM_2D;
    m_sol_pts.resize(m_nb_sol_pts,DIM_2D);
    m_flx_pts.resize(m_nb_flx_pts,DIM_2D);
    m_flx_pt_dirs.resize(m_nb_flx_pts);
    m_interp_grad_flx_to_sol_used_sol_pts.resize(m_nb_flx_pts);
    m_interp_sol_to_flx_used_sol_pts.resize(m_nb_flx_pts);
    m_interp_flx_to_sol_used_sol_pts.resize(m_nb_flx_pts);
    m_flx_pt_local_1d.resize(m_nb_flx_pts);
    m_sol_pt_local_1d.resize(m_nb_sol_pts,std::vector<Uint>(2));
    m_face_flx_pts.resize(4,std::vector<Uint>(m_local_1d.nb_sol_pts));
    m_flx_pt_sign.resize(m_nb_flx_pts,1.);

    for (Uint s_ksi=0; s_ksi<m_local_1d.nb_sol_pts; ++s_ksi)
    {
      for (Uint s_eta=0; s_eta<m_local_1d.nb_sol_pts; ++s_eta)
      {
        const Uint s = s_eta*m_local_1d.nb_sol_pts+s_ksi;

        m_sol_pts(s,KSI) = m_local_1d.sol_pts[s_ksi];
        m_sol_pts(s,ETA) = m_local_1d.sol_pts[s_eta];

        m_sol_pt_local_1d[s][KSI]=s_ksi;
        m_sol_pt_local_1d[s][ETA]=s_eta;
      }
    }

    for (Uint f_ksi=0; f_ksi<m_local_1d.nb_flx_pts; ++f_ksi)
    {
      for (Uint f_eta=0; f_eta<m_local_1d.nb_sol_pts; ++f_eta)
      {
        const Uint f = f_eta*m_local_1d.nb_flx_pts+f_ksi;
        m_flx_pts(f,KSI) = m_local_1d.flx_pts[f_ksi];
        m_flx_pts(f,ETA) = m_local_1d.sol_pts[f_eta];
        m_flx_pt_dirs[f].push_back(KSI);
        m_flx_pt_local_1d[f]=f_ksi;

        for (Uint s_ksi=0; s_ksi<m_local_1d.nb_sol_pts; ++s_ksi)
        {
          const Uint s = f_eta*m_local_1d.nb_sol_pts + s_ksi;

          if (std::abs( m_local_1d.interpolate_grad_flx_to_sol(s_ksi,f_ksi)) > 100*math::Consts::eps())
            m_interp_grad_flx_to_sol_used_sol_pts[f].push_back(s);

          if (std::abs( m_local_1d.interpolate_flx_to_sol(s_ksi,f_ksi)) > 100*math::Consts::eps())
            m_interp_flx_to_sol_used_sol_pts[f].push_back(s);

          if (std::abs( m_local_1d.interpolate_sol_to_flx(f_ksi,s_ksi)) > 100*math::Consts::eps())
            m_interp_sol_to_flx_used_sol_pts[f].push_back(s);
        }

        // f_eta is ignored as 1) the location may not be on faces; 2) it doesn't count as a face-point in the locally-1D line
        if (f_ksi==0)
        {
          m_face_flx_pts[KSI_NEG][m_local_1d.nb_sol_pts-1-f_eta]=f;
          m_flx_pt_sign[f]= -1.;
        }
        else if(f_ksi==m_local_1d.nb_flx_pts-1)
        {
          m_face_flx_pts[KSI_POS][f_eta]=f;
          m_flx_pt_sign[f]= +1.;
        }
        else
        {
          m_interior_flx_pts.push_back(f);
          m_flx_pt_sign[f]= +1.;
        }
      }
    }
    for (Uint f_ksi=0; f_ksi<m_local_1d.nb_sol_pts; ++f_ksi)
      for (Uint f_eta=0; f_eta<m_local_1d.nb_flx_pts; ++f_eta)
      {
        const Uint f = f_ksi*m_local_1d.nb_flx_pts+f_eta + m_local_1d.nb_sol_pts*m_local_1d.nb_flx_pts;
        m_flx_pts(f,KSI) = m_local_1d.sol_pts[f_ksi];
        m_flx_pts(f,ETA) = m_local_1d.flx_pts[f_eta];
        m_flx_pt_dirs[f].push_back(ETA);
        m_flx_pt_local_1d[f]=f_eta;

        for (Uint s_eta=0; s_eta<m_local_1d.nb_sol_pts; ++s_eta)
        {
          const Uint s = s_eta*m_local_1d.nb_sol_pts + f_ksi;

          if (std::abs( m_local_1d.interpolate_grad_flx_to_sol(s_eta,f_eta)) > 100*math::Consts::eps())
            m_interp_grad_flx_to_sol_used_sol_pts[f].push_back(s);

          if (std::abs( m_local_1d.interpolate_flx_to_sol(s_eta,f_eta)) > 100*math::Consts::eps())
            m_interp_flx_to_sol_used_sol_pts[f].push_back(s);

          if (std::abs( m_local_1d.interpolate_sol_to_flx(f_eta,s_eta)) > 100*math::Consts::eps())
            m_interp_sol_to_flx_used_sol_pts[f].push_back(s);
        }

        // f_ksi is ignored as 1) the location may not be on faces; 2) it doesn't count as a face-point in the locally-1D line
        if (f_eta==0)
        {
          m_face_flx_pts[ETA_NEG][f_ksi]=f;
          m_flx_pt_sign[f]=-1.;
        }
        else if(f_eta==m_local_1d.nb_flx_pts-1)
        {
          m_face_flx_pts[ETA_POS][m_local_1d.nb_sol_pts-1-f_ksi]=f;
          m_flx_pt_sign[f]=+1.;
        }
        else
        {
          m_interior_flx_pts.push_back(f);
          m_flx_pt_sign[f]=+1.;
        }

      }

    m_face_normals.resize(4,DIM_2D); m_face_normals.setZero();
    m_face_normals(KSI_NEG,KSI)=-1;
    m_face_normals(KSI_POS,KSI)=+1;
    m_face_normals(ETA_NEG,ETA)=-1;
    m_face_normals(ETA_POS,ETA)=+1;

  }


  virtual ~QuadLagrange1D() {}
  virtual void compute_value(const RealVector& local_coordinate, RealRowVector& value) const
  {
    cf3_assert(value.size()==nb_sol_pts());
    for (Uint s_ksi=0; s_ksi<m_local_1d.nb_sol_pts; ++s_ksi) {
      for (Uint s_eta=0; s_eta<m_local_1d.nb_sol_pts; ++s_eta) {
        Uint s = s_eta*m_local_1d.nb_sol_pts+s_ksi;
        value[s] = Lagrange::coeff(local_coordinate[KSI],m_local_1d.sol_pts,s_ksi) * Lagrange::coeff(local_coordinate[ETA],m_local_1d.sol_pts,s_eta);
      }
    }
  }
  virtual void compute_gradient(const RealVector& local_coordinate, RealMatrix& gradient) const
  {
    cf3_assert(gradient.rows()==DIM_2D);
    cf3_assert(gradient.cols()==nb_sol_pts());
    for (Uint s_ksi=0; s_ksi<m_local_1d.nb_sol_pts; ++s_ksi) {
      for (Uint s_eta=0; s_eta<m_local_1d.nb_sol_pts; ++s_eta) {
        Uint s = s_eta*m_local_1d.nb_sol_pts+s_ksi;
        gradient(KSI,s) = Lagrange::deriv_coeff(local_coordinate[KSI],m_local_1d.sol_pts,s_ksi) * Lagrange::coeff(local_coordinate[ETA],m_local_1d.sol_pts,s_eta);
        gradient(ETA,s) = Lagrange::coeff(local_coordinate[KSI],m_local_1d.sol_pts,s_ksi) * Lagrange::deriv_coeff(local_coordinate[ETA],m_local_1d.sol_pts,s_eta);
      }
    }
  }
  virtual void compute_flux_value(const Uint orientation, const RealVector& local_coordinate, RealRowVector& value) const
  {
    cf3_assert(value.size()==nb_flx_pts());
    value.setZero();
    switch (orientation)
    {
    case KSI:
      for (Uint f_ksi=0; f_ksi<m_local_1d.nb_flx_pts; ++f_ksi) {
        for (Uint f_eta=0; f_eta<m_local_1d.nb_sol_pts; ++f_eta) {
          const Uint f = f_eta*m_local_1d.nb_flx_pts+f_ksi;
          value[f] = Lagrange::coeff(local_coordinate[KSI],m_local_1d.flx_pts,f_ksi) * Lagrange::coeff(local_coordinate[ETA],m_local_1d.sol_pts,f_eta);
        }
      }
      break;
    case ETA:
      for (Uint f_ksi=0; f_ksi<m_local_1d.nb_sol_pts; ++f_ksi) {
        for (Uint f_eta=0; f_eta<m_local_1d.nb_flx_pts; ++f_eta)
        {
          const Uint f = f_ksi*m_local_1d.nb_flx_pts+f_eta + m_local_1d.nb_sol_pts*m_local_1d.nb_flx_pts;
          value[f] = Lagrange::coeff(local_coordinate[KSI],m_local_1d.sol_pts,f_ksi) * Lagrange::coeff(local_coordinate[ETA],m_local_1d.flx_pts,f_eta);
        }
      }
      break;
    }
  }
  virtual void compute_flux_derivative(const Uint orientation, const RealVector& local_coordinate, RealVector& derivative) const
  {
    cf3_assert(derivative.size()==nb_flx_pts());
    derivative.setZero();
    switch (orientation)
    {
    case KSI:
      for (Uint f_ksi=0; f_ksi<m_local_1d.nb_flx_pts; ++f_ksi) {
        for (Uint f_eta=0; f_eta<m_local_1d.nb_sol_pts; ++f_eta) {
          const Uint f = f_eta*m_local_1d.nb_flx_pts+f_ksi;
          derivative[f] = Lagrange::deriv_coeff(local_coordinate[KSI],m_local_1d.flx_pts,f_ksi) * Lagrange::coeff(local_coordinate[ETA],m_local_1d.sol_pts,f_eta);
        }
      }
      break;
    case ETA:
      for (Uint f_ksi=0; f_ksi<m_local_1d.nb_sol_pts; ++f_ksi) {
        for (Uint f_eta=0; f_eta<m_local_1d.nb_flx_pts; ++f_eta)
        {
          const Uint f = f_ksi*m_local_1d.nb_flx_pts+f_eta + m_local_1d.nb_sol_pts*m_local_1d.nb_flx_pts;
          derivative[f] = Lagrange::coeff(local_coordinate[KSI],m_local_1d.sol_pts,f_ksi) * Lagrange::deriv_coeff(local_coordinate[ETA],m_local_1d.flx_pts,f_eta);
        }
      }
      break;
    }
  }

  virtual const std::vector<Uint>& interpolate_grad_flx_to_sol_used_sol_pts(const Uint flx_pt, const Uint direction) const
  {
    // direction doesn't play a role because every flx_pt is uniquely defined for 1 direction
    return m_interp_grad_flx_to_sol_used_sol_pts[flx_pt];
  }
  virtual const std::vector<Uint>& interpolate_sol_to_flx_used_sol_pts(const Uint flx_pt) const
  {
    // direction doesn't play a role because every flx_pt is uniquely defined for 1 direction,
    // and if it would play a role, then just more solution points would be added in more directions
    return m_interp_sol_to_flx_used_sol_pts[flx_pt];
  }
  virtual const std::vector<Uint>& interpolate_flx_to_sol_used_sol_pts(const Uint flx_pt, const Uint direction) const
  {
    // direction plays a role because interpolation in different directions can be of a different order
    return m_interp_flx_to_sol_used_sol_pts[flx_pt];
  }
  virtual const Real& interpolate_grad_flx_to_sol_coeff(const Uint flx_pt, const Uint direction, const Uint sol_pt) const
  {
    // direction plays a role because every sol_pt is used in multiple directions
    return m_local_1d.interpolate_grad_flx_to_sol(m_sol_pt_local_1d[sol_pt][direction],m_flx_pt_local_1d[flx_pt]);
  }
  virtual const Real& interpolate_sol_to_flx_coeff(const Uint flx_pt, const Uint sol_pt) const
  {
    return m_local_1d.interpolate_sol_to_flx(m_flx_pt_local_1d[flx_pt],m_sol_pt_local_1d[sol_pt][m_flx_pt_dirs[flx_pt][0]]);
  }
  virtual const Real& interpolate_flx_to_sol_coeff(const Uint flx_pt, const Uint direction, const Uint sol_pt) const
  {
    return m_local_1d.interpolate_flx_to_sol(m_sol_pt_local_1d[sol_pt][direction],m_flx_pt_local_1d[flx_pt]);
  }
  virtual mesh::GeoShape::Type shape() const { return mesh::GeoShape::QUAD; }
  virtual Uint dimensionality() const { return DIM_2D; }
  virtual Uint nb_faces() const { return m_face_flx_pts.size(); }
  virtual Uint order() const { return m_order; }
  virtual Uint nb_sol_pts() const { return m_nb_sol_pts; }
  virtual Uint nb_flx_pts() const { return m_nb_flx_pts; }
  virtual const RealMatrix& sol_pts() const { return m_sol_pts; }
  virtual const RealMatrix& flx_pts() const { return m_flx_pts; }
  virtual const std::vector<Uint>& flx_pt_dirs(const Uint flx_pt) const { cf3_assert(flx_pt<nb_flx_pts()); return m_flx_pt_dirs[flx_pt]; }
  virtual const RealMatrix& face_normals() const { return m_face_normals; }
  virtual const std::vector<Uint>& interior_flx_pts() const { return m_interior_flx_pts; }
  virtual const std::vector<Uint>& face_flx_pts(const Uint face_idx) const { cf3_assert(face_idx<nb_faces()); return m_face_flx_pts[face_idx]; }
  virtual const Real& flx_pt_sign(const Uint flx_pt, const Uint dir) const { return m_flx_pt_sign[flx_pt]; }

private: // data

  Locally_1d                          m_local_1d;                             ///< holds 1D interpolation matrices
  Uint                                m_order;                                ///< Order of the solution shape function
  Uint                                m_nb_sol_pts;                           ///< Number of solution points
  Uint                                m_nb_flx_pts;                           ///< Number of flux points
  RealMatrix                          m_flx_pts;                              ///< Flux point coordinates
  RealMatrix                          m_sol_pts;                              ///< Solution point coordinates
  std::vector< std::vector<Uint> >    m_flx_pt_dirs;                          ///< Per flux point, the directions this flux point contributes to
  std::vector< std::vector<Uint> >    m_interp_grad_flx_to_sol_used_sol_pts;  ///< Per flux point, the solution points used in the derivatives
  std::vector< std::vector<Uint> >    m_interp_sol_to_flx_used_sol_pts;       ///< Per flux point, the solution points used to interpolate it
  std::vector< std::vector<Uint> >    m_interp_flx_to_sol_used_sol_pts;       ///< Per flux point, the solution points used in the interpolation
  std::vector<Uint>                   m_flx_pt_local_1d;                      ///< Mapping to a 1D flux point
  std::vector<std::vector<Uint> >     m_sol_pt_local_1d;                      ///< Mapping to a 1D solution point
  RealMatrix                          m_face_normals;                         ///< Rows are normals to faces according to FaceNumbering
  std::vector<Uint>                   m_interior_flx_pts;                     ///< Flux points that lie inside the cell, not on the faces
  std::vector<std::vector<Uint> >     m_face_flx_pts;                         ///< Flux points that on the cell faces
  std::vector<Real>                   m_flx_pt_sign;                          ///< Sign to be multiplied with computed flux in flx_pt in direction dir
};


////////////////////////////////////////////////////////////////////////////////

template<Uint P>
class sdm_API HexaLagrange1D : public sdm::ShapeFunction
{
public: // typedefs

  typedef boost::shared_ptr<HexaLagrange1D>       Ptr;
  typedef boost::shared_ptr<HexaLagrange1D const> ConstPtr;

private: // typedefs

  enum FaceNumbering {ZTA_NEG=0, ZTA_POS=1, ETA_NEG=2, KSI_POS=3, ETA_POS=4, KSI_NEG=5};

public: // functions

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW  ///< storing fixed-sized Eigen structures

  static std::string type_name() { return "HexaLagrange1D<"+common::to_str(P)+">"; }

  HexaLagrange1D(const std::string& name = type_name())
    : sdm::ShapeFunction(name),
      m_local_1d(P)
  {
    m_order = m_local_1d.nb_sol_pts-1;
    m_nb_sol_pts = m_local_1d.nb_sol_pts*m_local_1d.nb_sol_pts*m_local_1d.nb_sol_pts;
    m_nb_flx_pts = m_local_1d.nb_sol_pts*m_local_1d.nb_sol_pts*m_local_1d.nb_flx_pts*DIM_3D;
    m_sol_pts.resize(m_nb_sol_pts,DIM_3D);
    m_flx_pts.resize(m_nb_flx_pts,DIM_3D);
    m_flx_pt_dirs.resize(m_nb_flx_pts);
    m_interp_grad_flx_to_sol_used_sol_pts.resize(m_nb_flx_pts);
    m_interp_sol_to_flx_used_sol_pts.resize(m_nb_flx_pts);
    m_interp_flx_to_sol_used_sol_pts.resize(m_nb_flx_pts);
    m_flx_pt_local_1d.resize(m_nb_flx_pts);
    m_sol_pt_local_1d.resize(m_nb_sol_pts,std::vector<Uint>(DIM_3D));
    m_face_flx_pts.resize(6,std::vector<Uint>(m_local_1d.nb_sol_pts*m_local_1d.nb_sol_pts));
    m_flx_pt_sign.resize(m_nb_flx_pts,1.);

    // Define solution points
    for (Uint s_ksi=0; s_ksi<m_local_1d.nb_sol_pts; ++s_ksi)
    {
      for (Uint s_eta=0; s_eta<m_local_1d.nb_sol_pts; ++s_eta)
      {
        for (Uint s_zta=0; s_zta<m_local_1d.nb_sol_pts; ++s_zta)
        {
          const Uint s = (s_zta*m_local_1d.nb_sol_pts + s_eta)*m_local_1d.nb_sol_pts+s_ksi;

          m_sol_pts(s,KSI) = m_local_1d.sol_pts[s_ksi];
          m_sol_pts(s,ETA) = m_local_1d.sol_pts[s_eta];
          m_sol_pts(s,ZTA) = m_local_1d.sol_pts[s_zta];

          m_sol_pt_local_1d[s][KSI]=s_ksi;
          m_sol_pt_local_1d[s][ETA]=s_eta;
          m_sol_pt_local_1d[s][ZTA]=s_zta;
        }
      }
    }

    // define KSI direction flux points
    for (Uint f_ksi=0; f_ksi<m_local_1d.nb_flx_pts; ++f_ksi)
    {
      for (Uint f_eta=0; f_eta<m_local_1d.nb_sol_pts; ++f_eta)
      {
        for (Uint f_zta=0; f_zta<m_local_1d.nb_sol_pts; ++f_zta)
        {
          const Uint f = (f_zta*m_local_1d.nb_sol_pts + f_eta)*m_local_1d.nb_flx_pts+f_ksi;
          m_flx_pts(f,KSI) = m_local_1d.flx_pts[f_ksi];
          m_flx_pts(f,ETA) = m_local_1d.sol_pts[f_eta];
          m_flx_pts(f,ZTA) = m_local_1d.sol_pts[f_zta];
          m_flx_pt_dirs[f].push_back(KSI);
          m_flx_pt_local_1d[f]=f_ksi;

          for (Uint s_ksi=0; s_ksi<m_local_1d.nb_sol_pts; ++s_ksi)
          {
            const Uint s = (f_zta*m_local_1d.nb_sol_pts+f_eta)*m_local_1d.nb_sol_pts + s_ksi;

            if (std::abs( m_local_1d.interpolate_grad_flx_to_sol(s_ksi,f_ksi)) > 100*math::Consts::eps())
              m_interp_grad_flx_to_sol_used_sol_pts[f].push_back(s);

            if (std::abs( m_local_1d.interpolate_flx_to_sol(s_ksi,f_ksi)) > 100*math::Consts::eps())
              m_interp_flx_to_sol_used_sol_pts[f].push_back(s);

            if (std::abs( m_local_1d.interpolate_sol_to_flx(f_ksi,s_ksi)) > 100*math::Consts::eps())
              m_interp_sol_to_flx_used_sol_pts[f].push_back(s);
          }

          // f_eta and f_zta are ignored as 1) the location may not be on faces; 2) it doesn't count as a face-point in the locally-1D line
          if (f_ksi==0)
          {
            m_face_flx_pts[KSI_NEG][f_zta*m_local_1d.nb_sol_pts+f_eta]=f;
            m_flx_pt_sign[f]= -1.;
          }
          else if(f_ksi==m_local_1d.nb_flx_pts-1)
          {
            m_face_flx_pts[KSI_POS][f_zta*m_local_1d.nb_sol_pts+f_eta]=f;
            m_flx_pt_sign[f]= +1.;
          }
          else
          {
            m_interior_flx_pts.push_back(f);
            m_flx_pt_sign[f]= +1.;
          }
        }
      }
    }
    // define ETA direction flux points
    for (Uint f_ksi=0; f_ksi<m_local_1d.nb_sol_pts; ++f_ksi)
    {
      for (Uint f_eta=0; f_eta<m_local_1d.nb_flx_pts; ++f_eta)
      {
        for (Uint f_zta=0; f_zta<m_local_1d.nb_sol_pts; ++f_zta)
        {
          const Uint f = (f_zta*m_local_1d.nb_sol_pts + f_ksi)*m_local_1d.nb_flx_pts+f_eta + m_local_1d.nb_sol_pts*m_local_1d.nb_sol_pts*m_local_1d.nb_flx_pts;
          m_flx_pts(f,KSI) = m_local_1d.sol_pts[f_ksi];
          m_flx_pts(f,ETA) = m_local_1d.flx_pts[f_eta];
          m_flx_pts(f,ZTA) = m_local_1d.sol_pts[f_zta];

          m_flx_pt_dirs[f].push_back(ETA);
          m_flx_pt_local_1d[f]=f_eta;

          for (Uint s_eta=0; s_eta<m_local_1d.nb_sol_pts; ++s_eta)
          {
            const Uint s = (f_zta*m_local_1d.nb_sol_pts+s_eta)*m_local_1d.nb_sol_pts + f_ksi;

            if (std::abs( m_local_1d.interpolate_grad_flx_to_sol(s_eta,f_eta)) > 100*math::Consts::eps())
              m_interp_grad_flx_to_sol_used_sol_pts[f].push_back(s);

            if (std::abs( m_local_1d.interpolate_flx_to_sol(s_eta,f_eta)) > 100*math::Consts::eps())
              m_interp_flx_to_sol_used_sol_pts[f].push_back(s);

            if (std::abs( m_local_1d.interpolate_sol_to_flx(f_eta,s_eta)) > 100*math::Consts::eps())
              m_interp_sol_to_flx_used_sol_pts[f].push_back(s);
          }

          // f_ksi and f_zta are ignored as 1) the location may not be on faces; 2) it doesn't count as a face-point in the locally-1D line
          if (f_eta==0)
          {
            m_face_flx_pts[ETA_NEG][f_zta*m_local_1d.nb_sol_pts+f_ksi]=f;
            m_flx_pt_sign[f]=-1.;
          }
          else if(f_eta==m_local_1d.nb_flx_pts-1)
          {
            m_face_flx_pts[ETA_POS][f_zta*m_local_1d.nb_sol_pts+f_ksi]=f;
            m_flx_pt_sign[f]=+1.;
          }
          else
          {
            m_interior_flx_pts.push_back(f);
            m_flx_pt_sign[f]=+1.;
          }

        }
      }
    }
    // define ZTA direction flux points
    for (Uint f_ksi=0; f_ksi<m_local_1d.nb_sol_pts; ++f_ksi)
    {
      for (Uint f_eta=0; f_eta<m_local_1d.nb_sol_pts; ++f_eta)
      {
        for (Uint f_zta=0; f_zta<m_local_1d.nb_flx_pts; ++f_zta)
        {
          const Uint f = (f_eta*m_local_1d.nb_sol_pts + f_ksi)*m_local_1d.nb_flx_pts+f_zta + 2*m_local_1d.nb_sol_pts*m_local_1d.nb_sol_pts*m_local_1d.nb_flx_pts;
          m_flx_pts(f,KSI) = m_local_1d.sol_pts[f_ksi];
          m_flx_pts(f,ETA) = m_local_1d.sol_pts[f_eta];
          m_flx_pts(f,ZTA) = m_local_1d.flx_pts[f_zta];

          m_flx_pt_dirs[f].push_back(ZTA);
          m_flx_pt_local_1d[f]=f_zta;

          for (Uint s_zta=0; s_zta<m_local_1d.nb_sol_pts; ++s_zta)
          {
            const Uint s = (s_zta*m_local_1d.nb_sol_pts+f_eta)*m_local_1d.nb_sol_pts + f_ksi;

            if (std::abs( m_local_1d.interpolate_grad_flx_to_sol(s_zta,f_zta)) > 100*math::Consts::eps())
              m_interp_grad_flx_to_sol_used_sol_pts[f].push_back(s);

            if (std::abs( m_local_1d.interpolate_flx_to_sol(s_zta,f_zta)) > 100*math::Consts::eps())
              m_interp_flx_to_sol_used_sol_pts[f].push_back(s);

            if (std::abs( m_local_1d.interpolate_sol_to_flx(f_zta,s_zta)) > 100*math::Consts::eps())
              m_interp_sol_to_flx_used_sol_pts[f].push_back(s);
          }

          // f_ksi and f_eta are ignored as 1) the location may not be on faces; 2) it doesn't count as a face-point in the locally-1D line
          if (f_zta==0)
          {
            m_face_flx_pts[ZTA_NEG][f_eta*m_local_1d.nb_sol_pts+f_ksi]=f;
            m_flx_pt_sign[f]=-1.;
          }
          else if(f_zta==m_local_1d.nb_flx_pts-1)
          {
            m_face_flx_pts[ZTA_POS][f_eta*m_local_1d.nb_sol_pts+f_ksi]=f;
            m_flx_pt_sign[f]=+1.;
          }
          else
          {
            m_interior_flx_pts.push_back(f);
            m_flx_pt_sign[f]=+1.;
          }

        }
      }
    }

    m_face_normals.resize(6,DIM_3D);
    // set all components to zero
    m_face_normals.setZero();
    // change 1 component for each face in the right direction
    m_face_normals(KSI_NEG,KSI)=-1;
    m_face_normals(KSI_POS,KSI)=+1;
    m_face_normals(ETA_NEG,ETA)=-1;
    m_face_normals(ETA_POS,ETA)=+1;
    m_face_normals(ZTA_NEG,ZTA)=-1;
    m_face_normals(ZTA_POS,ZTA)=+1;

  }


  virtual ~HexaLagrange1D() {}
  virtual void compute_value(const RealVector& local_coordinate, RealRowVector& value) const
  {
    cf3_assert(value.size()==nb_sol_pts());
    for (Uint s_ksi=0; s_ksi<m_local_1d.nb_sol_pts; ++s_ksi) {
      for (Uint s_eta=0; s_eta<m_local_1d.nb_sol_pts; ++s_eta) {
        for (Uint s_zta=0; s_zta<m_local_1d.nb_sol_pts; ++s_zta)
        {
          const Uint s = (s_zta*m_local_1d.nb_sol_pts+s_eta)*m_local_1d.nb_sol_pts+s_ksi;
          value[s] = Lagrange::coeff(local_coordinate[KSI],m_local_1d.sol_pts,s_ksi)
                   * Lagrange::coeff(local_coordinate[ETA],m_local_1d.sol_pts,s_eta)
                   * Lagrange::coeff(local_coordinate[ZTA],m_local_1d.sol_pts,s_zta);
        }
      }
    }
  }
  virtual void compute_gradient(const RealVector& local_coordinate, RealMatrix& gradient) const
  {
    cf3_assert(gradient.rows()==DIM_3D);
    cf3_assert(gradient.cols()==nb_sol_pts());
    for (Uint s_ksi=0; s_ksi<m_local_1d.nb_sol_pts; ++s_ksi) {
      for (Uint s_eta=0; s_eta<m_local_1d.nb_sol_pts; ++s_eta) {
        for (Uint s_zta=0; s_zta<m_local_1d.nb_sol_pts; ++s_zta)
        {
          const Uint s = (s_zta*m_local_1d.nb_sol_pts+s_eta)*m_local_1d.nb_sol_pts+s_ksi;
          gradient(KSI,s) = Lagrange::deriv_coeff(local_coordinate[KSI],m_local_1d.sol_pts,s_ksi)
                          * Lagrange::coeff(local_coordinate[ETA],m_local_1d.sol_pts,s_eta)
                          * Lagrange::coeff(local_coordinate[ZTA],m_local_1d.sol_pts,s_zta);
          gradient(ETA,s) = Lagrange::coeff(local_coordinate[KSI],m_local_1d.sol_pts,s_ksi)
                          * Lagrange::deriv_coeff(local_coordinate[ETA],m_local_1d.sol_pts,s_eta)
                          * Lagrange::coeff(local_coordinate[ZTA],m_local_1d.sol_pts,s_zta);
          gradient(ZTA,s) = Lagrange::coeff(local_coordinate[KSI],m_local_1d.sol_pts,s_ksi)
                          * Lagrange::coeff(local_coordinate[ETA],m_local_1d.sol_pts,s_eta)
                          * Lagrange::deriv_coeff(local_coordinate[ZTA],m_local_1d.sol_pts,s_zta);
        }
      }
    }
  }
  virtual void compute_flux_value(const Uint orientation, const RealVector& local_coordinate, RealRowVector& value) const
  {
    cf3_assert(value.size()==nb_flx_pts());
    value.setZero();
    switch (orientation)
    {
    case KSI:
      for (Uint f_ksi=0; f_ksi<m_local_1d.nb_flx_pts; ++f_ksi) {
        for (Uint f_eta=0; f_eta<m_local_1d.nb_sol_pts; ++f_eta) {
          for (Uint f_zta=0; f_zta<m_local_1d.nb_sol_pts; ++f_zta) {
            const Uint f = (f_zta*m_local_1d.nb_sol_pts+f_eta)*m_local_1d.nb_flx_pts+f_ksi;
            value[f] = Lagrange::coeff(local_coordinate[KSI],m_local_1d.flx_pts,f_ksi)
                     * Lagrange::coeff(local_coordinate[ETA],m_local_1d.sol_pts,f_eta)
                     * Lagrange::coeff(local_coordinate[ZTA],m_local_1d.sol_pts,f_zta);
          }
        }
      }
      break;
    case ETA:
      for (Uint f_ksi=0; f_ksi<m_local_1d.nb_sol_pts; ++f_ksi) {
        for (Uint f_eta=0; f_eta<m_local_1d.nb_flx_pts; ++f_eta) {
          for (Uint f_zta=0; f_zta<m_local_1d.nb_sol_pts; ++f_zta)
          {
            const Uint f = (f_zta*m_local_1d.nb_sol_pts + f_ksi)*m_local_1d.nb_flx_pts+f_eta + m_local_1d.nb_sol_pts*m_local_1d.nb_sol_pts*m_local_1d.nb_flx_pts;
            value[f] = Lagrange::coeff(local_coordinate[KSI],m_local_1d.sol_pts,f_ksi)
                     * Lagrange::coeff(local_coordinate[ETA],m_local_1d.flx_pts,f_eta)
                     * Lagrange::coeff(local_coordinate[ZTA],m_local_1d.sol_pts,f_zta);
          }
        }
      }
      break;
    case ZTA:
      for (Uint f_ksi=0; f_ksi<m_local_1d.nb_sol_pts; ++f_ksi) {
        for (Uint f_eta=0; f_eta<m_local_1d.nb_sol_pts; ++f_eta) {
          for (Uint f_zta=0; f_zta<m_local_1d.nb_flx_pts; ++f_zta)
          {
            const Uint f = (f_eta*m_local_1d.nb_sol_pts + f_ksi)*m_local_1d.nb_flx_pts+f_zta + 2*m_local_1d.nb_sol_pts*m_local_1d.nb_sol_pts*m_local_1d.nb_flx_pts;
            value[f] = Lagrange::coeff(local_coordinate[KSI],m_local_1d.sol_pts,f_ksi)
                     * Lagrange::coeff(local_coordinate[ETA],m_local_1d.sol_pts,f_eta)
                     * Lagrange::coeff(local_coordinate[ZTA],m_local_1d.flx_pts,f_zta);
          }
        }
      }
      break;
    }
  }
  virtual void compute_flux_derivative(const Uint orientation, const RealVector& local_coordinate, RealVector& derivative) const
  {
    cf3_assert(derivative.size()==nb_flx_pts());
    derivative.setZero();

    switch (orientation)
    {
    case KSI:
      for (Uint f_ksi=0; f_ksi<m_local_1d.nb_flx_pts; ++f_ksi) {
        for (Uint f_eta=0; f_eta<m_local_1d.nb_sol_pts; ++f_eta) {
          for (Uint f_zta=0; f_zta<m_local_1d.nb_sol_pts; ++f_zta) {
            const Uint f = (f_zta*m_local_1d.nb_sol_pts+f_eta)*m_local_1d.nb_flx_pts+f_ksi;
            derivative[f] = Lagrange::deriv_coeff(local_coordinate[KSI],m_local_1d.flx_pts,f_ksi)
                          * Lagrange::coeff(local_coordinate[ETA],m_local_1d.sol_pts,f_eta)
                          * Lagrange::coeff(local_coordinate[ZTA],m_local_1d.sol_pts,f_zta);
          }
        }
      }
      break;
    case ETA:
      for (Uint f_ksi=0; f_ksi<m_local_1d.nb_sol_pts; ++f_ksi) {
        for (Uint f_eta=0; f_eta<m_local_1d.nb_flx_pts; ++f_eta) {
          for (Uint f_zta=0; f_zta<m_local_1d.nb_sol_pts; ++f_zta)
          {
            const Uint f = (f_zta*m_local_1d.nb_sol_pts + f_ksi)*m_local_1d.nb_flx_pts+f_eta + m_local_1d.nb_sol_pts*m_local_1d.nb_sol_pts*m_local_1d.nb_flx_pts;
            derivative[f] = Lagrange::coeff(local_coordinate[KSI],m_local_1d.sol_pts,f_ksi)
                          * Lagrange::deriv_coeff(local_coordinate[ETA],m_local_1d.flx_pts,f_eta)
                          * Lagrange::coeff(local_coordinate[ZTA],m_local_1d.sol_pts,f_zta);
          }
        }
      }
      break;
    case ZTA:
      for (Uint f_ksi=0; f_ksi<m_local_1d.nb_sol_pts; ++f_ksi) {
        for (Uint f_eta=0; f_eta<m_local_1d.nb_sol_pts; ++f_eta) {
          for (Uint f_zta=0; f_zta<m_local_1d.nb_flx_pts; ++f_zta)
          {
            const Uint f = (f_eta*m_local_1d.nb_sol_pts + f_ksi)*m_local_1d.nb_flx_pts+f_zta + 2*m_local_1d.nb_sol_pts*m_local_1d.nb_sol_pts*m_local_1d.nb_flx_pts;
            derivative[f] = Lagrange::coeff(local_coordinate[KSI],m_local_1d.sol_pts,f_ksi)
                          * Lagrange::coeff(local_coordinate[ETA],m_local_1d.sol_pts,f_eta)
                          * Lagrange::deriv_coeff(local_coordinate[ZTA],m_local_1d.flx_pts,f_zta);
          }
        }
      }
      break;
    }

  }

  virtual const std::vector<Uint>& interpolate_grad_flx_to_sol_used_sol_pts(const Uint flx_pt, const Uint direction) const
  {
    // direction doesn't play a role because every flx_pt is uniquely defined for 1 direction
    return m_interp_grad_flx_to_sol_used_sol_pts[flx_pt];
  }
  virtual const std::vector<Uint>& interpolate_sol_to_flx_used_sol_pts(const Uint flx_pt) const
  {
    // direction doesn't play a role because every flx_pt is uniquely defined for 1 direction,
    // and if it would play a role, then just more solution points would be added in more directions
    return m_interp_sol_to_flx_used_sol_pts[flx_pt];
  }
  virtual const std::vector<Uint>& interpolate_flx_to_sol_used_sol_pts(const Uint flx_pt, const Uint direction) const
  {
    // direction plays a role because interpolation in different directions can be of a different order
    return m_interp_flx_to_sol_used_sol_pts[flx_pt];
  }
  virtual const Real& interpolate_grad_flx_to_sol_coeff(const Uint flx_pt, const Uint direction, const Uint sol_pt) const
  {
    // direction plays a role because every sol_pt is used in multiple directions
    return m_local_1d.interpolate_grad_flx_to_sol(m_sol_pt_local_1d[sol_pt][direction],m_flx_pt_local_1d[flx_pt]);
  }
  virtual const Real& interpolate_sol_to_flx_coeff(const Uint flx_pt, const Uint sol_pt) const
  {
    return m_local_1d.interpolate_sol_to_flx(m_flx_pt_local_1d[flx_pt],m_sol_pt_local_1d[sol_pt][m_flx_pt_dirs[flx_pt][0]]);
  }
  virtual const Real& interpolate_flx_to_sol_coeff(const Uint flx_pt, const Uint direction, const Uint sol_pt) const
  {
    return m_local_1d.interpolate_flx_to_sol(m_sol_pt_local_1d[sol_pt][direction],m_flx_pt_local_1d[flx_pt]);
  }
  virtual mesh::GeoShape::Type shape() const { return mesh::GeoShape::HEXA; }
  virtual Uint dimensionality() const { return DIM_3D; }
  virtual Uint nb_faces() const { return m_face_flx_pts.size(); }
  virtual Uint order() const { return m_order; }
  virtual Uint nb_sol_pts() const { return m_nb_sol_pts; }
  virtual Uint nb_flx_pts() const { return m_nb_flx_pts; }
  virtual const RealMatrix& sol_pts() const { return m_sol_pts; }
  virtual const RealMatrix& flx_pts() const { return m_flx_pts; }
  virtual const std::vector<Uint>& flx_pt_dirs(const Uint flx_pt) const { cf3_assert(flx_pt<nb_flx_pts()); return m_flx_pt_dirs[flx_pt]; }
  virtual const RealMatrix& face_normals() const { return m_face_normals; }
  virtual const std::vector<Uint>& interior_flx_pts() const { return m_interior_flx_pts; }
  virtual const std::vector<Uint>& face_flx_pts(const Uint face_idx) const { cf3_assert(face_idx<nb_faces()); return m_face_flx_pts[face_idx]; }
  virtual const Real& flx_pt_sign(const Uint flx_pt, const Uint dir) const { return m_flx_pt_sign[flx_pt]; }

private: // data

  Locally_1d                          m_local_1d;                             ///< holds 1D interpolation matrices
  Uint                                m_order;                                ///< Order of the solution shape function
  Uint                                m_nb_sol_pts;                           ///< Number of solution points
  Uint                                m_nb_flx_pts;                           ///< Number of flux points
  RealMatrix                          m_flx_pts;                              ///< Flux point coordinates
  RealMatrix                          m_sol_pts;                              ///< Solution point coordinates
  std::vector< std::vector<Uint> >    m_flx_pt_dirs;                          ///< Per flux point, the directions this flux point contributes to
  std::vector< std::vector<Uint> >    m_interp_grad_flx_to_sol_used_sol_pts;  ///< Per flux point, the solution points used in the derivatives
  std::vector< std::vector<Uint> >    m_interp_sol_to_flx_used_sol_pts;       ///< Per flux point, the solution points used to interpolate it
  std::vector< std::vector<Uint> >    m_interp_flx_to_sol_used_sol_pts;       ///< Per flux point, the solution points used in the interpolation
  std::vector<Uint>                   m_flx_pt_local_1d;                      ///< Mapping to a 1D flux point
  std::vector<std::vector<Uint> >     m_sol_pt_local_1d;                      ///< Mapping to a 1D solution point
  RealMatrix                          m_face_normals;                         ///< Rows are normals to faces according to FaceNumbering
  std::vector<Uint>                   m_interior_flx_pts;                     ///< Flux points that lie inside the cell, not on the faces
  std::vector<std::vector<Uint> >     m_face_flx_pts;                         ///< Flux points that on the cell faces
  std::vector<Real>                   m_flx_pt_sign;                          ///< Sign to be multiplied with computed flux in flx_pt in direction dir
};


////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

#endif // cf3_sdm_ShapeFunctionT_hpp
