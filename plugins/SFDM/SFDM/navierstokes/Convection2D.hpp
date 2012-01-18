// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_navierstokes_Convection2D_hpp
#define cf3_SFDM_navierstokes_Convection2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "SFDM/ConvectiveTerm.hpp"
#include "SFDM/navierstokes/LibNavierStokes.hpp"
#include "Physics/NavierStokes/Cons2D.hpp"
#include "Physics/NavierStokes/Roe2D.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {
namespace navierstokes {

////////////////////////////////////////////////////////////////////////////////

class SFDM_navierstokes_API Convection2D : public ConvectiveTerm< ConvectiveTermPointData<4u,2u> >
{
private:
  typedef physics::NavierStokes::Cons2D PHYS;
  typedef physics::NavierStokes::Roe2D  ROE;

public:
  static std::string type_name() { return "Convection2D"; }
  Convection2D(const std::string& name) : ConvectiveTerm< ConvectiveTermPointData<4u,2u> >(name)
  {
  }

  virtual ~Convection2D() {}

  virtual void initialize()
  {
    ConvectiveTerm< ConvectiveTermPointData<4u,2u> >::initialize();
    physical_model().handle<PHYS::MODEL>()->set_gas_constants(p);
    physical_model().handle<PHYS::MODEL>()->set_gas_constants(p_left);
    physical_model().handle<PHYS::MODEL>()->set_gas_constants(p_right);
  }

  virtual void compute_analytical_flux(ConvectiveTermPointData<4u,2u>& data, const PHYS::MODEL::GeoV& unit_normal,
                                       PHYS::MODEL::SolV& flux, Real& wave_speed)
  {
    PHYS::compute_properties(dummy_coords, data.solution , dummy_grads, p);
    PHYS::flux(p, unit_normal, flux);
    PHYS::flux_jacobian_eigen_values(p, unit_normal, eigenvalues);
    wave_speed = eigenvalues.cwiseAbs().maxCoeff();
  }

  virtual void compute_numerical_flux(ConvectiveTermPointData<4u,2u>& left, ConvectiveTermPointData<4u,2u>& right, const PHYS::MODEL::GeoV& unit_normal,
                                      PHYS::MODEL::SolV& flux, Real& wave_speed)
  {
//    cf3_assert(left.coord == right.coord);
    // Compute left and right properties
    PHYS::compute_properties(left.coord,left.solution,dummy_grads,p_left);
    PHYS::compute_properties(right.coord,right.solution,dummy_grads,p_right);

    // Compute the Roe averaged properties
    // Roe-average = standard average of the Roe-parameter vectors
    ROE::compute_variables(p_left,  roe_left );
    ROE::compute_variables(p_right, roe_right);
    roe_avg.noalias() = 0.5*(roe_left+roe_right); // Roe-average is result
    ROE::compute_properties(left.coord, roe_avg, dummy_grads, p);

    // Compute absolute jacobian using Roe averaged properties
    PHYS::flux_jacobian_eigen_structure(p,unit_normal,right_eigenvectors,left_eigenvectors,eigenvalues);
    abs_jacobian.noalias() = right_eigenvectors * eigenvalues.cwiseAbs().asDiagonal() * left_eigenvectors;

    // Compute left and right fluxes
    PHYS::flux(p_left , unit_normal, flux_left);
    PHYS::flux(p_right, unit_normal, flux_right);

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

#endif // cf3_SFDM_navierstokes_Convection2D_hpp
