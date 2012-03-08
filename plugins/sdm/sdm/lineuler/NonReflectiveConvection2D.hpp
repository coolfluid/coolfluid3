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
#include "math/Functions.hpp"

#include "sdm/ConvectiveTerm.hpp"
#include "sdm/lineuler/LibLinEuler.hpp"
#include "Physics/LinEuler/Cons2D.hpp"

/// If this is defined, then also the Aplus wave is modified!
//#define LILLA

// ********* UNDEFINE THE EQUATION FOR Amin TO USE AT BC *************

/// Normal characteristic equations:
/// dS/dt     + U0n grad(S) = 0
/// dOmega/dt + U0n grad(Omega) + 0.5 c0s grad(Aplus+Amin) = 0
/// dAplus/dt + (U0n+c0n) grad(Aplus) + c0s grad(Omega) = 0
/// dAmin/dt  + (U0n-c0n) grad(Aplus) + c0s grad(Omega) = 0

/// The idea is to change the equation for Amin at the boundary nodes

/// Try1
/// dAmin/dt + (U0n+c0n) grad(Amin) - c0s grad(Omega) = 0
/// This means:
///   dA/dt     + (U0n+c0n) . grad(A) = 0
///   domega/dt + (U0n+c0n) . grad(omega) + 2 c0s grad(Omega) = 0
// Here all that is required is change the sign of soundspeed
//#define AplusAmin -p.c
// or equivalent:
//#define Aomega -p.c

// **** RECOMMENDED, seems better with 0 sound-speed *** //
/// Try2
/// dAmin/dt + U0n grad(Amin) = 0
/// This means:
///   dA/dt     + U0n grad(A)     + c0n grad(Aplus) + c0s grad(Omega) = 0
///   domega/dt + U0n grad(omega) + c0n grad(Aplus) + c0s grad(Omega) = 0
// Here all that is required is set soundspeed to zero
#define AplusAmin 0
// or equivalent:
//#define Aomega 0

/// Try3
/// dAmin/dt + U0n grad(Amin) - c0n grad(Aplus) - c0s grad(Omega) = 0
/// This means:
///   dA/dt     + U0n grad(A) = 0
///   domega/dt + U0n grad(omega) + c0n grad(A) = 0
//#define ConvectATry2 p.c

/// dAmin/dt + (U0n+c0n) grad(Amin) + c0s grad(Omega) = 0
/// This means:
///   dA/dt     + (U0n+c0n) . grad(A) + 2 c0s grad(Omega) = 0
///   domega/dt + (U0n+c0n) . grad(omega) = 0
// Here new system is developed with correct sound-speed
//#define Aminconvection p.c

/// dAmin/dt + U0n grad(Amin) - c0n grad(Aplus) - c0s grad(Omega) = 0
/// This means:
///   dA/dt     + (U0n) . grad(A) = 0
///   domega/dt + (U0n+c0n) . grad(omega) + c0n grad(A) + 2 c0s grad(Omega) = 0
// Here new system is developed with correct sound-speed
//#define Aconvection p.c

/// dAmin/dt = 0
/// This means:
///   dA/dt     = (U0n+c0n)/2 grad(A+omega) + c0s grad(Omega) = 0
///   domega/dt = (U0n+c0n)/2 grad(A+omega) + c0s grad(Omega) = 0
// Here we just set the flux to zero
//#define Hedstrom 0.

/// dAplus/dt + U0n grad(Aplus) + c0s grad(Omega) = 0
/// dAmin/dt  + U0n grad(Amin)  + c0s grad(Omega) = 0
/// This means:
///   dA/dt     + U0n grad(A) + 2 c0s grad(Omega) = 0
///   domega/dt + U0n grad(omega) = 0
//#define TwoWaves p.c


// This will not correct the physics, what is imposed on the right is what you get
//#define NOTHING

// Things that can be imposed on the right:
//#define Extrapolation
//#define Aminzero

// *******************************************************************

#if defined(LILLA)
#undef AplusAmin
#undef Aomega
#undef Aminconvection
#undef Aconvection
#undef Hedstrom
#undef ConvectATry2
#define TwoWaves p.c
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace lineuler {

////////////////////////////////////////////////////////////////////////////////

class sdm_lineuler_API NonReflectiveConvection2D : public ConvectiveTerm< PhysDataBase<4u,2u> >
{
private:
  typedef physics::LinEuler::Cons2D PHYS;

  enum {ENTR=0, SHEAR=1, APLUS=2, AMIN=3};

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
    Real c0 = math::Consts::real_max();

#if defined(Aomega)
    c0 = Aomega;
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
        U0n, U0n, U0n+c0, U0n-c0;
    abs_jacobian.noalias() = right_eigenvectors * eigenvalues.cwiseAbs().asDiagonal() * left_eigenvectors;
    cons_to_bc(left.solution,char_normal,bc_sol_left);
    cons_to_bc(right.solution,char_normal,bc_sol_right);
    bc_flux.noalias() =  0.5*jacobian*(bc_sol_left+bc_sol_right) - 0.5*abs_jacobian*(bc_sol_right-bc_sol_left);
    bc_to_cons(bc_flux,char_normal,flux);
#elif defined(AplusAmin)
    c0=AplusAmin;
    jacobian <<
         U0n,      0,               0,             0,
         0,        U0n,             0.5*c0*sxn,    0.5*c0*sxn,
         0,        c0*sxn,          U0n+c0*ncxn,   0,
         0,        c0*sxn,          0,             U0n-c0*ncxn;

    right_eigenvectors <<
        1,  0,          0,              0,
        0,  0.5*ncxn,   0.5*sxn,       -0.5*sxn,
        0, -0.5*sxn,    0.5*(1.+ncxn),  0.5*(1.-ncxn),
        0,  0.5*sxn,    0.5*(1.-ncxn),  0.5*(1.+ncxn);
    left_eigenvectors <<
        1,   0,         0,              0,
        0,   2*ncxn,   -sxn,            sxn,
        0,   sxn,       0.5*(1.+ncxn),  0.5*(1.-ncxn),
        0,  -sxn,       0.5*(1.-ncxn),  0.5*(1.+ncxn);
    eigenvalues <<
        U0n, U0n, U0n+c0, U0n-c0;
    abs_jacobian.noalias() = right_eigenvectors * eigenvalues.cwiseAbs().asDiagonal() * left_eigenvectors;
    cons_to_char(left.solution,char_normal,bc_sol_left);
    cons_to_char(right.solution,char_normal,bc_sol_right);
    bc_flux.noalias() =  0.5*jacobian*(bc_sol_left+bc_sol_right) - 0.5*abs_jacobian*(bc_sol_right-bc_sol_left);
    char_to_cons(bc_flux,char_normal,flux);
#elif defined(Aminconvection)

    c0 = Aminconvection;

    const Real dummy = 1.;// math::Consts::real_max();
    jacobian <<
         U0n,      0,               0,             0,
         0,        U0n,             0.5*c0*sxn,    0.5*c0*sxn,
         0,        c0*sxn,          U0n+c0*ncxn,   0,
         0,        c0*sxn,          0,             U0n+c0*ncxn;

    right_eigenvectors <<
        dummy,  dummy,   dummy,   dummy,
        dummy,  dummy,   dummy,   dummy,
        dummy,  dummy,   dummy,   dummy,
        0,      0,       1,      -1;

    Real t = std::sqrt(1.+3.*sxn*sxn);
    cf3_assert(t!=0);
    left_eigenvectors <<
        1,   0,       0,    0,
        0,   0,       1,   -1,
        0,   sxn/t,   0,    0.5*(1.+ncxn/t),
        0,   sxn/t,   0,   -0.5*(1.-ncxn/t);
    eigenvalues <<
                   U0n,
                   U0n + c0*ncxn,
                   U0n + 0.5*c0*(ncxn+t),
                   U0n + 0.5*c0*(ncxn-t);
    abs_jacobian.noalias() = right_eigenvectors * eigenvalues.cwiseAbs().asDiagonal() * left_eigenvectors;
    cons_to_char(left.solution,char_normal,bc_sol_left);
    cons_to_char(right.solution,char_normal,bc_sol_right);
    bc_flux.noalias() =  0.5*jacobian*(bc_sol_left+bc_sol_right) - 0.5*abs_jacobian*(bc_sol_right-bc_sol_left);
    char_to_cons(bc_flux,char_normal,flux);

#elif defined(Aconvection)

    c0 = Aconvection;

    const Real dummy = 1.;// math::Consts::real_max();
    jacobian <<
         U0n,      0,               0,             0,
         0,        U0n,             0.5*c0*sxn,    0.5*c0*sxn,
         0,        c0*sxn,          U0n+c0*ncxn,   0,
         0,       -c0*sxn,          -c0*ncxn,      U0n;

    right_eigenvectors <<
        dummy,  dummy,   dummy,   dummy,
        dummy,  dummy,   dummy,   dummy,
        dummy,  dummy,   dummy,   dummy,
        0,      1,       0.5,     0.5;

    Real t = std::sqrt(1.+3.*sxn*sxn);
    cf3_assert(t!=0);
    left_eigenvectors <<
        1,   0,         0,    0,
        0,   0,         0.5, -0.5,
        0,  -2.*sxn/t,  -0.5*(3.*ncxn+t)/t,  0.5*(t-1.)/t,
        0,   2.*sxn/t,   0.5*(3.*ncxn-t)/t,  0.5*(t+1.)/t;
    eigenvalues <<
                   U0n,
                   U0n,
                   U0n + 0.5*c0*(ncxn+t),
                   U0n + 0.5*c0*(ncxn-t);
    abs_jacobian.noalias() = right_eigenvectors * eigenvalues.cwiseAbs().asDiagonal() * left_eigenvectors;
    cons_to_char(left.solution,char_normal,bc_sol_left);
    cons_to_char(right.solution,char_normal,bc_sol_right);
    bc_flux.noalias() =  0.5*jacobian*(bc_sol_left+bc_sol_right) - 0.5*abs_jacobian*(bc_sol_right-bc_sol_left);
    char_to_cons(bc_flux,char_normal,flux);
#elif defined(Hedstrom)

    c0  = 0.;
    U0n = 0.;
    const Real dummy = 1.;// math::Consts::real_max();
    jacobian <<
         U0n,      0,               0,             0,
         0,        U0n,             0.5*c0*sxn,    0.5*c0*sxn,
         0,        c0*sxn,          U0n+c0*ncxn,   0,
         0,       -c0*sxn,          -c0*ncxn,      U0n;

    // Any zero-eigensystem will do (just copy from before)
    right_eigenvectors <<
        dummy,  dummy,   dummy,   dummy,
        dummy,  dummy,   dummy,   dummy,
        dummy,  dummy,   dummy,   dummy,
        0,      1,       0.5,     0.5;

    Real t = std::sqrt(1.+3.*sxn*sxn);
    cf3_assert(t!=0);
    left_eigenvectors <<
        1,   0,         0,    0,
        0,   0,         0.5, -0.5,
        0,  -2.*sxn/t,  -0.5*(3.*ncxn+t)/t,  0.5*(t-1.)/t,
        0,   2.*sxn/t,   0.5*(3.*ncxn-t)/t,  0.5*(t+1.)/t;
    eigenvalues <<
                   U0n,
                   U0n,
                   U0n + 0.5*c0*(ncxn+t),
                   U0n + 0.5*c0*(ncxn-t);
    abs_jacobian.noalias() = right_eigenvectors * eigenvalues.cwiseAbs().asDiagonal() * left_eigenvectors;
    cons_to_char(left.solution,char_normal,bc_sol_left);
    cons_to_char(right.solution,char_normal,bc_sol_right);
    bc_flux.noalias() =  0.5*jacobian*(bc_sol_left+bc_sol_right) - 0.5*abs_jacobian*(bc_sol_right-bc_sol_left);
    char_to_cons(bc_flux,char_normal,flux);
#elif defined(TwoWaves)
    c0=TwoWaves;
    jacobian <<
         U0n,      0,               0,             0,
         0,        U0n,             0.5*c0*sxn,    0.5*c0*sxn,
         0,        c0*sxn,          U0n,           0,
         0,        c0*sxn,          0,             U0n;
    right_eigenvectors <<
        1,  0,      0,    0,
        0,  0,     -1,   -1,
        0,  0.5,    1,   -1,
        0, -0.5,    1,   -1;
    left_eigenvectors <<
        1,   0,     0,      0,
        0,   0,     1,     -1,
        0,  -0.5,   0.25,   0.25,
        0,  -0.5,  -0.25,  -0.25;
    eigenvalues <<
        U0n, U0n, U0n-c0*sxn, U0n+c0*sxn;   // !!!!!!!!! <-- NOTICE multiplication with sxn!!!!!!

    abs_jacobian.noalias() = right_eigenvectors * eigenvalues.cwiseAbs().asDiagonal() * left_eigenvectors;
    cons_to_char(left.solution,char_normal,bc_sol_left);
    cons_to_char(right.solution,char_normal,bc_sol_right);
    bc_flux.noalias() =  0.5*jacobian*(bc_sol_left+bc_sol_right) - 0.5*abs_jacobian*(bc_sol_right-bc_sol_left);
    char_to_cons(bc_flux,char_normal,flux);
#elif defined(ConvectATry2)
    c0=ConvectATry2;
    jacobian <<
         U0n,      0,               0,             0,
         0,        U0n,             0.5*c0*sxn,   -0.5*c0*sxn,
         0,        c0*sxn,          U0n+c0*ncxn,   0,
         0,       -c0*sxn,         -c0*ncxn,       U0n;
    Real dummy=0.;
    right_eigenvectors <<
        dummy,   dummy,   dummy,   dummy,
        dummy,   dummy,   dummy,   dummy,
        0,       1,      -1,      -1,
        0,       1,       1,       1;
    Real t = std::sqrt(1+3*sxn*sxn);
    left_eigenvectors <<
        1,   0,       0,                   0,
        0,   0,       0.5,                 0.5,
        0,  -sxn/t,  -0.25*(1+3*ncxn/t),   0.25*(1-3*ncxn/t),
        0,   sxn/t,  -0.25*(1-3*ncxn/t),   0.25*(1+3*ncxn/t);
    eigenvalues <<
        U0n,  U0n,  U0n+0.5*c0*(ncxn+t),  U0n+0.5*c0*(ncxn-t);

    abs_jacobian.noalias() = right_eigenvectors * eigenvalues.cwiseAbs().asDiagonal() * left_eigenvectors;
    cons_to_char(left.solution,char_normal,bc_sol_left);
    cons_to_char(right.solution,char_normal,bc_sol_right);
    bc_flux.noalias() =  0.5*jacobian*(bc_sol_left+bc_sol_right) - 0.5*abs_jacobian*(bc_sol_right-bc_sol_left);
    char_to_cons(bc_flux,char_normal,flux);
#else
#error nothing defined
#endif
    cf3_assert(c0!=math::Consts::real_max());
    wave_speed = std::abs(U0n);
  }

  virtual void compute_analytical_non_reflective_flux(PhysData& data, const RealVectorNDIM& unit_normal, const RealVectorNDIM& char_normal,
                                                      RealVectorNEQS& flux, Real& wave_speed)
  {
    PHYS::compute_properties(data.coord, data.solution , dummy_grads, p);

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
    Real c0 = math::Consts::real_max();

#if defined(Aomega)

    c0 = Aomega;

    jacobian <<
         U0n,      0,               0,             0,
         0,        U0n,             0.5*c0*sxn,    0,
         0,        2.*c0*sxn,       U0n,           c0*ncxn,
         0,        0,               c0*ncxn,       U0n;
    cons_to_bc(data.solution,char_normal,bc_sol);
    bc_flux = jacobian*bc_sol;
    bc_to_cons(bc_flux,char_normal,flux);
#elif defined(AplusAmin)

    c0 = AplusAmin;

    jacobian <<
         U0n,      0,               0,             0,
         0,        U0n,             0.5*c0*sxn,    0.5*c0*sxn,
         0,        c0*sxn,          U0n+c0*ncxn,   0,
         0,        c0*sxn,          0,             U0n-c0*ncxn;
    cons_to_char(data.solution,char_normal,bc_sol);
    bc_flux = jacobian*bc_sol;
    char_to_cons(bc_flux,char_normal,flux);
#elif defined(Aminconvection)

    c0 = Aminconvection;

    jacobian <<
         U0n,      0,               0,             0,
         0,        U0n,             0.5*c0*sxn,    0.5*c0*sxn,
         0,        c0*sxn,          U0n+c0*ncxn,   0,
         0,        c0*sxn,          0,             U0n+c0*ncxn;
    cons_to_char(data.solution,char_normal,bc_sol);
    bc_flux = jacobian*bc_sol;
    char_to_cons(bc_flux,char_normal,flux);
#elif defined(Aconvection)

    c0 = Aconvection;

    jacobian <<
         U0n,      0,               0,             0,
         0,        U0n,             0.5*c0*sxn,    0.5*c0*sxn,
         0,        c0*sxn,          U0n+c0*ncxn,   0,
         0,       -c0*sxn,          -c0*ncxn,      U0n;
    cons_to_char(data.solution,char_normal,bc_sol);
    bc_flux = jacobian*bc_sol;
    char_to_cons(bc_flux,char_normal,flux);
#elif defined(Hedstrom)

    c0  = 0.;
    U0n = 0.;

    // Any zero-matrix will do
    jacobian <<
         U0n,      0,               0,             0,
         0,        U0n,             0.5*c0*sxn,    0.5*c0*sxn,
         0,        c0*sxn,          U0n+c0*ncxn,   0,
         0,       -c0*sxn,          -c0*ncxn,      U0n;
    cons_to_char(data.solution,char_normal,bc_sol);
    bc_flux = jacobian*bc_sol;
    char_to_cons(bc_flux,char_normal,flux);
#elif defined(TwoWaves)
    c0 = TwoWaves;
    jacobian <<
         U0n,      0,               0,             0,
         0,        U0n,             0.5*c0*sxn,    0.5*c0*sxn,
         0,        c0*sxn,          U0n,           0,
         0,        c0*sxn,          0,             U0n;
    cons_to_char(data.solution,char_normal,bc_sol);
    bc_flux = jacobian*bc_sol;
    char_to_cons(bc_flux,char_normal,flux);
#elif defined(ConvectATry2)
    c0=ConvectATry2;
    jacobian <<
                U0n,      0,               0,             0,
                0,        U0n,             0.5*c0*sxn,   -0.5*c0*sxn,
                0,        c0*sxn,          U0n+c0*ncxn,   0,
                0,       -c0*sxn,         -c0*ncxn,       U0n;
    cons_to_char(data.solution,char_normal,bc_sol);
    bc_flux = jacobian*bc_sol;
    char_to_cons(bc_flux,char_normal,flux);
#else
#error nothing defined
#endif
    cf3_assert(c0!=math::Consts::real_max());
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
