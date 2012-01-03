// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_scalar_LinearAdvection2D_hpp
#define cf3_SFDM_scalar_LinearAdvection2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include "SFDM/ConvectiveTerm.hpp"
#include "SFDM/scalar/LibScalar.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {
namespace scalar {

////////////////////////////////////////////////////////////////////////////////

class SFDM_scalar_API LinearAdvection2D : public ConvectiveTerm< ConvectiveTermPointData<1u,2u> >
{
public:
  static std::string type_name() { return "LinearAdvection2D"; }
  LinearAdvection2D(const std::string& name) : ConvectiveTerm(name)
  {
    m_advection_speed.resize(2u);
    m_advection_speed[XX]= 1.;
    m_advection_speed[YY]= 1.;

    options().add_option("advection_speed",m_advection_speed).link_to(&m_advection_speed);
  }
  virtual ~LinearAdvection2D() {}

//  virtual void compute_analytical_flux(const RealVector2& unit_normal)
//  {
//    Real A = (unit_normal[XX]*m_advection_speed[XX]+unit_normal[YY]*m_advection_speed[YY]);
//    flx_pt_flux[flx_pt] = A*flx_pt_solution->get()[flx_pt];
//    flx_pt_wave_speed[flx_pt][0] = std::abs(A);
//  }

//  virtual void compute_numerical_flux(const RealVector2& unit_normal)
//  {
//    RealVector1& left  = flx_pt_solution->get()[flx_pt];
//    RealVector1& right = flx_pt_neighbour_solution->get()[neighbour_flx_pt];
//    Real A = (m_advection_speed[XX]*unit_normal[XX]+m_advection_speed[YY]*unit_normal[YY]);
//    flx_pt_flux[flx_pt] = 0.5 * A*(left + right) - 0.5 * std::abs(A)*(right - left);
//    flx_pt_wave_speed[flx_pt][0] = std::abs(A);
//  }

private:
  std::vector<Real> m_advection_speed;
};

////////////////////////////////////////////////////////////////////////////////

} // scalar
} // SFDM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_SFDM_scalar_LinearAdvection2D_hpp
