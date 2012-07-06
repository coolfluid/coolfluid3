// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_lineuler_BCSubsonicOutletThompson2D_hpp
#define cf3_sdm_lineuler_BCSubsonicOutletThompson2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "sdm/BCWeak.hpp"
#include "sdm/lineuler/LibLinEuler.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace lineuler {

////////////////////////////////////////////////////////////////////////////////

class sdm_lineuler_API BCSubsonicOutletThompson2D : public BCWeak< PhysDataBase<4u,2u> >
{
public:
  static std::string type_name() { return "BCSubsonicOutletThompson2D"; }
  BCSubsonicOutletThompson2D(const std::string& name) : BCWeak< PhysData >(name)
  {

    m_c0 = 1.;
    options().add("c0",m_c0)
        .description("Uniform mean sound speed")
        .link_to(&m_c0)
        .mark_basic();

  }

  virtual ~BCSubsonicOutletThompson2D() {}

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:

  virtual void initialize()
  {
    BCWeak< PhysData >::initialize();
    flx_pt_plane_jacobian_normal = shared_caches().get_cache< FluxPointPlaneJacobianNormal<NDIM> >();
    flx_pt_plane_jacobian_normal->options().set("space",solution_field().dict().handle<mesh::Dictionary>());
  }

  virtual void set_inner_cell()
  {
    BCWeak< PhysData >::set_inner_cell();
    flx_pt_plane_jacobian_normal->cache(cell_entities,cell_idx);
  }
  virtual void unset_inner_cell()
  {
    BCWeak< PhysData >::unset_inner_cell();
    flx_pt_plane_jacobian_normal->get().unlock();
  }

  virtual void compute_solution(const PhysData &inner_cell_data, const RealVectorNDIM &boundary_face_normal, RealVectorNEQS &boundary_face_solution)
  {
    RealVectorNEQS char_cell_sol;
    RealVectorNEQS char_bdry_sol;

    const RealVectorNDIM& unit_normal = flx_pt_plane_jacobian_normal->get().plane_unit_normal[cell_flx_pt] * flx_pt_plane_jacobian_normal->get().sf->flx_pt_sign(cell_flx_pt);
    Real nx = unit_normal[XX];
    Real ny = unit_normal[YY];

    cons_to_char(inner_cell_data.solution,unit_normal,char_cell_sol);
    const Real& S     = char_cell_sol[0];
    const Real& Omega = char_cell_sol[1];
    const Real& Aplus = char_cell_sol[2];
    const Real& Amin  = char_cell_sol[3];

    char_bdry_sol[0] =  S;
    char_bdry_sol[1] =  Omega;
    char_bdry_sol[2] =  Aplus;
    char_bdry_sol[3] =  0.; // Amin zero

    char_to_cons(char_bdry_sol,unit_normal,boundary_face_solution);
  }

  void cons_to_char(const RealVectorNEQS& conservative, const RealVectorNDIM& characteristic_normal, RealVectorNEQS& characteristic)
  {
    const Real& rho   = conservative[0];
    const Real& rho0u = conservative[1];
    const Real& rho0v = conservative[2];
    const Real& press = conservative[3];
    const Real& nx    = characteristic_normal[XX];
    const Real& ny    = characteristic_normal[YY];
    Real& S     = characteristic[0];
    Real& Omega = characteristic[1];
    Real& Aplus = characteristic[2];
    Real& Amin  = characteristic[3];

    S     =  rho - press/(m_c0*m_c0);
    Omega =  ny*rho0u - nx*rho0v;
    Aplus =  nx*rho0u + ny*rho0v + press/m_c0;
    Amin  = -nx*rho0u - ny*rho0v + press/m_c0;
  }

  void char_to_cons(const RealVectorNEQS& characteristic, const RealVectorNDIM& characteristic_normal, RealVectorNEQS& conservative)
  {
    const Real& S     = characteristic[0];
    const Real& Omega = characteristic[1];
    const Real& Aplus = characteristic[2];
    const Real& Amin  = characteristic[3];
    const Real& nx    = characteristic_normal[XX];
    const Real& ny    = characteristic_normal[YY];
    Real& rho   = conservative[0];
    Real& rho0u = conservative[1];
    Real& rho0v = conservative[2];
    Real& press = conservative[3];

    const Real A     = Aplus+Amin;
    const Real omega = Aplus-Amin;

    rho   =  S + 0.5*A/m_c0;
    rho0u =  ny*Omega + 0.5*nx*omega;
    rho0v = -nx*Omega + 0.5*ny*omega;
    press =  0.5*m_c0*A;
  }

  Handle<CacheT<FluxPointPlaneJacobianNormal<NDIM> > > flx_pt_plane_jacobian_normal;

  Real m_c0;
};

////////////////////////////////////////////////////////////////////////////////

} // lineuler
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_lineuler_BCSubsonicOutletThompson2D_hpp
