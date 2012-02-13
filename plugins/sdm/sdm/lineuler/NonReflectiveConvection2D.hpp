// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_lineuler_NonReflectiveConvection2D_hpp
#define cf3_sdm_lineuler_NonReflectiveConvection2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "sdm/ConvectiveTerm.hpp"
#include "sdm/lineuler/LibLinEuler.hpp"
#include "Physics/LinEuler/Cons2D.hpp"

//#define NON_REFL 1
#undef NON_REFL

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace lineuler {

////////////////////////////////////////////////////////////////////////////////

class sdm_lineuler_API NonReflectiveConvection2D : public ConvectiveTerm< PhysDataBase<4u,2u> >
{
private:
  typedef physics::LinEuler::Cons2D PHYS;

public:
  static std::string type_name() { return "NonReflectiveConvection2D"; }
  NonReflectiveConvection2D(const std::string& name) : ConvectiveTerm< PhysData >(name)
  {
    p.gamma = 1.4;
    options().add_option("gamma",p.gamma)
        .description("Specific heat reatio")
        .attach_trigger( boost::bind( &NonReflectiveConvection2D::config_constants, this) );

    p.rho0 = 1.;
    options().add_option("rho0",p.rho0)
        .description("Uniform mean density")
        .attach_trigger( boost::bind( &NonReflectiveConvection2D::config_constants, this) );

    p.u0.setZero();
    std::vector<Real> U0(p.u0.size());
    for (Uint d=0; d<U0.size(); ++d)
      U0[d] = p.u0[d];
    options().add_option("U0",U0)
        .description("Uniform mean velocity")
        .attach_trigger( boost::bind( &NonReflectiveConvection2D::config_constants, this) );

    options().add_option("p0",p.P0)
        .description("Uniform mean pressure")
        .attach_trigger( boost::bind( &NonReflectiveConvection2D::config_constants, this) );

    config_constants();
    boundary_face_nb = -1;
  }

  void config_constants()
  {
    p.gamma = options().option("gamma").value<Real>();
    p.rho0  = options().option("rho0").value<Real>();
    p.P0  = options().option("p0").value<Real>();

    p.inv_rho0 = 1./p.rho0;

    p.c=sqrt(p.gamma*p.P0*p.inv_rho0);
    p.inv_c = 1./p.c;

    std::vector<Real> U0 = options().option("U0").value<std::vector<Real> >();
    for (Uint d=0; d<U0.size(); ++d)
      p.u0[d] = U0[d];
  }

  virtual ~NonReflectiveConvection2D() {}


  virtual void compute_analytical_flux(PhysData& data, const RealVectorNDIM& unit_normal,
                                       RealVectorNEQS& flux, Real& wave_speed)
  {
    PHYS::compute_properties(data.coord, data.solution , dummy_grads, p);

    const Real u0n = p.u0[XX] * unit_normal[XX] +
                     p.u0[YY] * unit_normal[YY];

    const Real un = p.u * unit_normal[XX] +
                    p.v * unit_normal[YY];

    flux[0] = u0n*p.rho   + p.rho0*un;
    flux[1] = u0n*p.rho0u + p.p*unit_normal[XX];
    flux[2] = u0n*p.rho0v + p.p*unit_normal[YY];
    flux[3] = u0n*p.p     + p.rho0*un*p.c*p.c;

  }

  virtual void compute_numerical_flux(PhysData& left, PhysData& right, const RealVectorNDIM& unit_normal,
                                      RealVectorNEQS& flux, Real& wave_speed)
  {
    // Compute the averaged properties
    sol_avg.noalias() = 0.5*(left.solution+right.solution);
    PHYS::compute_properties(left.coord, sol_avg, dummy_grads, p);

    const Real nx = unit_normal[XX];
    const Real ny = unit_normal[YY];

    // state is not used as Linearized Euler is, well, linear

    const Real u0n = p.u0[XX] * nx + p.u0[YY]   * ny;

    const Real inv_c2  = p.inv_c*p.inv_c;

    right_eigenvectors <<
          1.,      0.,      0.5*p.inv_c,  0.5*p.inv_c,
          0.,      ny,      0.5*nx,      -0.5*nx,
          0.,     -nx,      0.5*ny,      -0.5*ny,
          0.,      0.,      0.5*p.c,      0.5*p.c;

    left_eigenvectors <<
          1.,      0.,      0.,          -inv_c2,
          0.,      ny,     -nx,           0,
          0.,      nx,      ny,           p.inv_c,
          0.,     -nx,     -ny,           p.inv_c;

    eigenvalues << u0n , u0n, u0n + p.c,  u0n - p.c;


    abs_jacobian.noalias() = right_eigenvectors * eigenvalues.cwiseAbs().asDiagonal() * left_eigenvectors;

    PHYS::compute_properties(left.coord, left.solution, dummy_grads, p);
    Real un = p.u * nx + p.v * ny;

    flux_left[0] = u0n*p.rho   + p.rho0*un;
    flux_left[1] = u0n*p.rho0u + p.p*nx;
    flux_left[2] = u0n*p.rho0v + p.p*ny;
    flux_left[3] = u0n*p.p     + p.rho0*un*p.c*p.c;

    PHYS::compute_properties(right.coord, right.solution, dummy_grads, p);
    un = p.u * nx + p.v * ny;

    flux_right[0] = u0n*p.rho   + p.rho0*un;
    flux_right[1] = u0n*p.rho0u + p.p*nx;
    flux_right[2] = u0n*p.rho0v + p.p*ny;
    flux_right[3] = u0n*p.p     + p.rho0*un*p.c*p.c;


    // flux = central flux - upwind flux
    flux.noalias() = 0.5*(flux_left + flux_right);
    flux.noalias() -= 0.5*abs_jacobian*(right.solution-left.solution);
    wave_speed = eigenvalues.cwiseAbs().maxCoeff();
  }


  virtual void compute_numerical_non_reflective_flux(PhysData& left, PhysData& right, const RealVectorNDIM& unit_normal,const RealVectorNDIM& char_normal,
                                      RealVectorNEQS& flux, Real& wave_speed)
  {
    // Compute the averaged properties
    sol_avg.noalias() = 0.5*(left.solution+right.solution);
    PHYS::compute_properties(left.coord, sol_avg, dummy_grads, p);

    Real nx = unit_normal[XX];
    Real ny = unit_normal[YY];
    Real ncx = char_normal[XX];
    Real ncy = char_normal[YY];
    Real U0n = (p.u0[XX]*nx + p.u0[YY]*ny);

    RealVectorNDIM s;
    s[XX] =  ncy;
    s[YY] = -ncx;
    Real sxn = unit_normal.transpose() * s;
    Real ncxn = unit_normal.transpose() * char_normal;

    /// This line is the hack that makes things not reflect
    Real c0 = -p.c;

    jacobian <<
         U0n,      0,               0,             0,
         0,        U0n,             0.5*c0*sxn,    0,
         0,        2.*c0*sxn,       U0n,           c0*ncxn,
         0,        0,               c0*ncxn,       U0n;

    right_eigenvectors <<
        1,  0,          0,         0,
        0,  0.5*ncxn,   0.5*sxn,   0.5*sxn,
        0,  0,          1,        -1,
        0, -sxn,        ncxn,      ncxn;
    left_eigenvectors <<
                         1,  0,       0,     0,
                         0,  2*ncxn,  0,    -sxn,
                         0,  sxn,     0.5,   0.5*ncxn,
                         0,  sxn,    -0.5,   0.5*ncxn;
    eigenvalues <<
        U0n, U0n, U0n+c0*ncxn, U0n-c0*ncxn;


    abs_jacobian.noalias() = right_eigenvectors * eigenvalues.cwiseAbs().asDiagonal() * left_eigenvectors;


    cons_to_bc(left.solution,char_normal,bc_sol_left);
    cons_to_bc(right.solution,char_normal,bc_sol_right);

    wave_speed = std::abs(U0n);

    // flux = central flux - upwind flux
    bc_flux.noalias() =  0.5*jacobian*(bc_sol_left+bc_sol_right) - 0.5*abs_jacobian*(bc_sol_right-bc_sol_left);

    bc_to_cons(bc_flux,char_normal,flux);

  }

  virtual void compute_analytical_non_reflective_flux(PhysData& data, const RealVectorNDIM& unit_normal, const RealVectorNDIM& char_normal,
                                                      RealVectorNEQS& flux, Real& wave_speed)
  {
    PHYS::compute_properties(data.coord, data.solution , dummy_grads, p);
    cons_to_bc(data.solution,char_normal,bc_sol);

    Real nx = unit_normal[XX];
    Real ny = unit_normal[YY];
    Real ncx = char_normal[XX];
    Real ncy = char_normal[YY];
    Real U0n = (p.u0[XX]*nx + p.u0[YY]*ny);

    RealVectorNDIM s;
    s[XX] =  ncy;
    s[YY] = -ncx;
    Real sxn = unit_normal.transpose() * s;
    Real ncxn = unit_normal.transpose() * char_normal;

    /// This line is the hack that makes things not reflect
    Real c0 = -p.c;
    jacobian <<
         U0n,      0,               0,             0,
         0,        U0n,             0.5*c0*sxn,    0,
         0,        2.*c0*sxn,       U0n,           c0*ncxn,
         0,        0,               c0*ncxn,       U0n;

    bc_flux = jacobian*bc_sol;
    bc_to_cons(bc_flux,char_normal,flux);
    wave_speed = p.u0.transpose()*char_normal;
  }

  void compute_fluxes(std::vector<RealVectorNEQS>& flux_in_flx_pts);
  void compute_non_reflective_fluxes(std::vector<RealVectorNEQS>& flux_in_flx_pts);

  virtual void execute();

  void set_boundary_face(const Uint face_nb)
  {
    boundary_face_nb = face_nb;
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

  virtual void cons_to_bc(const RealVectorNEQS& conservative, const RealVectorNDIM& characteristic_normal, RealVectorNEQS& bc_vars)
  {
    cons_to_bc(conservative,characteristic_normal,bc_vars,p);
  }

  virtual void bc_to_cons(const RealVectorNEQS& bc_vars, const RealVectorNDIM& characteristic_normal, RealVectorNEQS& conservative)
  {
    bc_to_cons(bc_vars,characteristic_normal,conservative,p);
  }

  RealVectorNDIM characteristic_normal() const
  {
    Uint flx_pt = elem->get().sf->face_flx_pts(boundary_face_nb)[0];
    RealVectorNDIM char_normal = flx_pt_plane_jacobian_normal->get().plane_unit_normal[flx_pt] * elem->get().sf->flx_pt_sign(flx_pt);
    return char_normal;
  }

  static void cons_to_bc(const RealVectorNEQS& conservative, const RealVectorNDIM& characteristic_normal, RealVectorNEQS& bc_vars, const PHYS::MODEL::Properties& p)
  {

    const Real& rho   = conservative[0];
    const Real& rho0u = conservative[1];
    const Real& rho0v = conservative[2];
    const Real& press = conservative[3];
    const Real& nx    = characteristic_normal[XX];
    const Real& ny    = characteristic_normal[YY];
    Real& S     = bc_vars[0];
    Real& Omega = bc_vars[1];
    Real& A     = bc_vars[2];
    Real& omega = bc_vars[3];

    S     = rho - press*p.inv_c*p.inv_c;
    Omega = ny*rho0u - nx*rho0v;
    A     = 2. * press*p.inv_c;
    omega = 2. * (nx*rho0u + ny*rho0v);
  }

  static void bc_to_cons(const RealVectorNEQS& bc_vars, const RealVectorNDIM& characteristic_normal, RealVectorNEQS& conservative, const PHYS::MODEL::Properties& p)
  {
    const Real& S     = bc_vars[0];
    const Real& Omega = bc_vars[1];
    const Real& A     = bc_vars[2];
    const Real& omega = bc_vars[3];
    const Real& nx    = characteristic_normal[XX];
    const Real& ny    = characteristic_normal[YY];
    Real& rho   = conservative[0];
    Real& rho0u = conservative[1];
    Real& rho0v = conservative[2];
    Real& press = conservative[3];

    rho   =  S + 0.5*p.inv_c*A;
    rho0u =  ny*Omega + 0.5*nx*omega;
    rho0v = -nx*Omega + 0.5*ny*omega;
    press =  0.5*p.c*A;
  }


  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:

  PHYS::MODEL::Properties p;
  PHYS::MODEL::Properties p_left;
  PHYS::MODEL::Properties p_right;

  PHYS::MODEL::SolM dummy_grads;
  PHYS::MODEL::GeoV dummy_coords;

  PHYS::MODEL::SolV sol_avg;

  PHYS::MODEL::SolV flux_left;
  PHYS::MODEL::SolV flux_right;

  PHYS::MODEL::SolV eigenvalues;
  PHYS::MODEL::JacM right_eigenvectors;
  PHYS::MODEL::JacM left_eigenvectors;
  PHYS::MODEL::JacM  abs_jacobian;
  PHYS::MODEL::JacM  jacobian;

  RealVectorNEQS bc_flux;
  RealVectorNEQS bc_flux_left;
  RealVectorNEQS bc_flux_right;
  RealVectorNEQS bc_sol;
  RealVectorNEQS bc_sol_left;
  RealVectorNEQS bc_sol_right;
  RealVectorNEQS bc_sol_avg;

  int boundary_face_nb;
};

////////////////////////////////////////////////////////////////////////////////

} // lineuler
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_lineuler_NonReflectiveConvection2D_hpp
