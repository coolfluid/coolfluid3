// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_navierstokes_Convection1D_hpp
#define cf3_SFDM_navierstokes_Convection1D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "SFDM/ConvectiveTerm.hpp"
#include "SFDM/navierstokes/LibNavierStokes.hpp"
#include "Physics/NavierStokes/Cons1D.hpp"
#include "Physics/NavierStokes/Roe1D.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {
namespace navierstokes {

////////////////////////////////////////////////////////////////////////////////

class SFDM_navierstokes_API Convection1D : public ConvectiveTerm<3u,1u>
{
private:
  typedef physics::NavierStokes::Cons1D PHYS;
  typedef physics::NavierStokes::Roe1D  ROE;

public:
  static std::string type_name() { return "Convection1D"; }
  Convection1D(const std::string& name) : ConvectiveTerm(name)
  {
    p.gamma = 1.4;
    p.gamma_minus_1 = p.gamma-1.;
    p.R = 287.05;
    options().add_option("gamma",p.gamma).attach_trigger(boost::bind(&Convection1D::configure_gamma, this));
    options().add_option("R",p.R).link_to(&p.R);
  }

  void configure_gamma()
  {
    p.gamma = options().option("gamma").value<Real>();
    p.gamma_minus_1 = p.gamma - 1.;

    p_left.gamma = p.gamma;
    p_left.gamma_minus_1 = p.gamma_minus_1;

    p_right.gamma = p.gamma;
    p_right.gamma_minus_1 = p.gamma_minus_1;
  }

  virtual ~Convection1D() {}

  virtual void compute_analytical_flux(const PHYS::MODEL::GeoV& unit_normal)
  {
    PHYS::compute_properties(dummy_coords, flx_pt_solution->get()[flx_pt] , dummy_grads, p);
    PHYS::flux(p, unit_normal, flx_pt_flux[flx_pt]);
    PHYS::flux_jacobian_eigen_values(p, unit_normal, eigenvalues);
    flx_pt_wave_speed[flx_pt][0] = eigenvalues.cwiseAbs().maxCoeff();
  }

  virtual void compute_numerical_flux(const PHYS::MODEL::GeoV& unit_normal)
  {
    PHYS::MODEL::SolV& left  = flx_pt_solution->get()[flx_pt];
    PHYS::MODEL::SolV& right = flx_pt_neighbour_solution->get()[neighbour_flx_pt];

    // Compute left and right properties
    PHYS::compute_properties(dummy_coords,left,dummy_grads,p_left);
    PHYS::compute_properties(dummy_coords,right,dummy_grads,p_right);

    // Compute the Roe averaged properties
    // Roe-average = standard average of the Roe-parameter vectors
    ROE::compute_variables(p_left,  roe_left );
    ROE::compute_variables(p_right, roe_right);
    roe_avg.noalias() = 0.5*(roe_left+roe_right); // Roe-average is result
    ROE::compute_properties(dummy_coords, roe_avg, dummy_grads, p);

    // Compute absolute jacobian using Roe averaged properties
    PHYS::flux_jacobian_eigen_structure(p,unit_normal,right_eigenvectors,left_eigenvectors,eigenvalues);
    abs_jacobian.noalias() = right_eigenvectors * eigenvalues.cwiseAbs().asDiagonal() * left_eigenvectors;

    // Compute left and right fluxes
    PHYS::flux(p_left , unit_normal, flux_left);
    PHYS::flux(p_right, unit_normal, flux_right);

    // flux = central flux - upwind flux
    flx_pt_flux[flx_pt].noalias() = 0.5*(flux_left + flux_right);
    flx_pt_flux[flx_pt].noalias() -= 0.5*abs_jacobian*(right-left);
    flx_pt_wave_speed[flx_pt][0] = eigenvalues.cwiseAbs().maxCoeff();
  }

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:

  PHYS::MODEL::Properties p;
  PHYS::MODEL::Properties p_left;
  PHYS::MODEL::Properties p_right;

  PHYS::MODEL::SolM dummy_grads;
  PHYS::MODEL::GeoV dummy_coords;

  PHYS::MODEL::SolV roe_avg;
  PHYS::MODEL::SolV roe_left;
  PHYS::MODEL::SolV roe_right;

  PHYS::MODEL::SolV flux_left;
  PHYS::MODEL::SolV flux_right;

  PHYS::MODEL::SolV eigenvalues;
  PHYS::MODEL::JacM right_eigenvectors;
  PHYS::MODEL::JacM left_eigenvectors;
  PHYS::MODEL::JacM  abs_jacobian;
};

////////////////////////////////////////////////////////////////////////////////

} // navierstokes
} // SFDM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_SFDM_navierstokes_Convection1D_hpp
