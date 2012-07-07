// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_lineuler_Convection2D_hpp
#define cf3_sdm_lineuler_Convection2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "sdm/ConvectiveTerm.hpp"
#include "sdm/lineuler/LibLinEuler.hpp"
#include "Physics/LinEuler/Cons2D.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace lineuler {

////////////////////////////////////////////////////////////////////////////////

class sdm_lineuler_API Convection2D : public ConvectiveTerm< PhysDataBase<4u,2u> >
{
private:
  typedef physics::LinEuler::Cons2D PHYS;

public:
  static std::string type_name() { return "Convection2D"; }
  Convection2D(const std::string& name) : ConvectiveTerm< PhysData >(name)
  {
    p.gamma = 1.4;
    options().add("gamma",p.gamma)
        .description("Specific heat reatio")
        .attach_trigger( boost::bind( &Convection2D::config_constants, this) )
        .mark_basic();

    p.rho0 = 1.;
    options().add("rho0",p.rho0)
        .description("Uniform mean density")
        .attach_trigger( boost::bind( &Convection2D::config_constants, this) )
        .mark_basic();

    p.u0.setZero();
    std::vector<Real> U0(p.u0.size());
    for (Uint d=0; d<U0.size(); ++d)
      U0[d] = p.u0[d];
    options().add("U0",U0)
        .description("Uniform mean velocity")
        .attach_trigger( boost::bind( &Convection2D::config_constants, this) )
        .mark_basic();

    options().add("p0",p.P0)
        .description("Uniform mean pressure")
        .attach_trigger( boost::bind( &Convection2D::config_constants, this) )
        .mark_basic();

    config_constants();
  }

  void config_constants()
  {
    p.gamma = options().value<Real>("gamma");
    p.rho0  = options().value<Real>("rho0");
    p.P0  = options().value<Real>("p0");

    p.inv_rho0 = 1./p.rho0;

    p.c=sqrt(p.gamma*p.P0*p.inv_rho0);
    p.inv_c = 1./p.c;

    std::vector<Real> U0 = options().value<std::vector<Real> >("U0");
    for (Uint d=0; d<U0.size(); ++d)
      p.u0[d] = U0[d];
  }

  virtual ~Convection2D() {}


  virtual void compute_analytical_flux(PhysData& data, const RealVectorNDIM& unit_normal,
                                       RealVectorNEQS& flux, Real& wave_speed)
  {
    PHYS::compute_properties(data.coord, data.solution , dummy_grads, p);
    PHYS::flux(p, unit_normal, flux);
    PHYS::flux_jacobian_eigen_values(p, unit_normal, eigenvalues);
    wave_speed = eigenvalues.cwiseAbs().maxCoeff();
  }

  virtual void compute_numerical_flux(PhysData& left, PhysData& right, const RealVectorNDIM& unit_normal,
                                      RealVectorNEQS& flux, Real& wave_speed)
  {
    // Compute left and right fluxes
    PHYS::compute_properties(left.coord, left.solution, dummy_grads, p);
    PHYS::flux(p , unit_normal, flux_left);

    PHYS::compute_properties(left.coord, right.solution, dummy_grads, p);
    PHYS::flux(p , unit_normal, flux_right);

    // Compute the averaged properties
    sol_avg.noalias() = 0.5*(left.solution+right.solution);
    PHYS::compute_properties(left.coord, sol_avg, dummy_grads, p);

    // Compute absolute jacobian using averaged properties
    PHYS::flux_jacobian_eigen_structure(p,unit_normal,right_eigenvectors,left_eigenvectors,eigenvalues);
    abs_jacobian.noalias() = right_eigenvectors * eigenvalues.cwiseAbs().asDiagonal() * left_eigenvectors;

    // flux = central flux - upwind flux
    flux.noalias() = 0.5*(flux_left + flux_right);
    flux.noalias() -= 0.5*abs_jacobian*(right.solution-left.solution);
    wave_speed = eigenvalues.cwiseAbs().maxCoeff();
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
};

////////////////////////////////////////////////////////////////////////////////

} // lineuler
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_lineuler_Convection2D_hpp
