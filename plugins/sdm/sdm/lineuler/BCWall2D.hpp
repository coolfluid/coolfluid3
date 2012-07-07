// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_lineuler_BCWall2D_hpp
#define cf3_sdm_lineuler_BCWall2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "Physics/LinEuler/LinEuler2D.hpp"
#include "sdm/BCWeak.hpp"
#include "sdm/lineuler/LibLinEuler.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace lineuler {

////////////////////////////////////////////////////////////////////////////////

class sdm_lineuler_API BCWall2D : public BCWeak< PhysDataBase<4u,2u> >
{
public:
  static std::string type_name() { return "BCWall2D"; }
  BCWall2D(const std::string& name) : BCWeak< PhysData >(name)
  {
    p.gamma = 1.4;
    options().add("gamma",p.gamma)
        .description("Specific heat reatio")
        .attach_trigger( boost::bind( &BCWall2D::config_constants, this) );

    p.rho0 = 1.;
    options().add("rho0",p.rho0)
        .description("Uniform mean density")
        .attach_trigger( boost::bind( &BCWall2D::config_constants, this) );

    p.u0.setZero();
    std::vector<Real> U0(p.u0.size());
    for (Uint d=0; d<U0.size(); ++d)
      U0[d] = p.u0[d];
    options().add("U0",U0)
        .description("Uniform mean velocity")
        .attach_trigger( boost::bind( &BCWall2D::config_constants, this) );

    options().add("p0",p.P0)
        .description("Uniform mean pressure")
        .attach_trigger( boost::bind( &BCWall2D::config_constants, this) );

    config_constants();
  }

  void config_constants()
  {
    p.gamma = options().value<Real>("gamma");
    p.rho0  = options().value<Real>("rho0");
    p.P0    = options().value<Real>("p0");

    p.inv_rho0 = 1./p.rho0;

    p.c=sqrt(p.gamma*p.P0*p.inv_rho0);
    p.inv_c = 1./p.c;
  }

  virtual ~BCWall2D() {}

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

    outward_normal = flx_pt_plane_jacobian_normal->get().plane_unit_normal[cell_flx_pt] * flx_pt_plane_jacobian_normal->get().sf->flx_pt_sign(cell_flx_pt);
    Real nx = outward_normal[XX];
    Real ny = outward_normal[YY];

    boundary_face_solution = inner_cell_data.solution;

//  COMMENTED HERE IS A STANDARD FORM NOT BASED ON CHARACTERISTIC THEORY
// velocity on the inside of the face
    rho0u[XX] = inner_cell_data.solution[1];
    rho0u[YY] = inner_cell_data.solution[2];

    // velocity in outward_normal direction
    rho0u_normal = rho0u.transpose()*outward_normal;

    // Modify velocity to become the outside velocity of the face,
    // and being the mirror of the inside velocity
    rho0u.noalias() -= 2.*rho0u_normal*outward_normal;

    boundary_face_solution[1] = rho0u[XX];
    boundary_face_solution[2] = rho0u[YY];

//    cons_to_char(inner_cell_data.solution,outward_normal,char_cell_sol);
//    const Real& S     = char_cell_sol[0];
//    const Real& Omega = char_cell_sol[1];
//    const Real& Aplus = char_cell_sol[2];
//    const Real& Amin  = char_cell_sol[3];

//    char_bdry_sol[0] =  S;
//    char_bdry_sol[1] =  Omega;
//    char_bdry_sol[2] =  Aplus;
//    char_bdry_sol[3] =  Aplus; // Amin = Aplus is the magic here

//    char_to_cons(char_bdry_sol,outward_normal,boundary_face_solution);
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

    S     =  rho - press*p.inv_c*p.inv_c;
    Omega =  ny*rho0u - nx*rho0v;
    Aplus =  nx*rho0u + ny*rho0v + press*p.inv_c;
    Amin  = -nx*rho0u - ny*rho0v + press*p.inv_c;
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

    rho   =  S + 0.5*p.inv_c*A;
    rho0u =  ny*Omega + 0.5*nx*omega;
    rho0v = -nx*Omega + 0.5*ny*omega;
    press =  0.5*p.c*A;
  }

  Handle<CacheT<FluxPointPlaneJacobianNormal<NDIM> > > flx_pt_plane_jacobian_normal;
  physics::LinEuler::LinEuler2D::Properties p;


  RealVectorNDIM rho0u;
  RealVectorNDIM outward_normal;
  Real rho0u_normal;

};

////////////////////////////////////////////////////////////////////////////////

} // lineuler
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_lineuler_BCWall2D_hpp
