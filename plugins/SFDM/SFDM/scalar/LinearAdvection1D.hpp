// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_scalar_LinearAdvection1D_hpp
#define cf3_SFDM_scalar_LinearAdvection1D_hpp

////////////////////////////////////////////////////////////////////////////////

#include "SFDM/ConvectiveTerm.hpp"
#include "SFDM/scalar/LibScalar.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {
namespace scalar {

////////////////////////////////////////////////////////////////////////////////

class SFDM_scalar_API LinearAdvection1D : public ConvectiveTerm< ConvectiveTermPointData<1u,1u> >
{
public:
  static std::string type_name() { return "LinearAdvection1D"; }
  LinearAdvection1D(const std::string& name) : ConvectiveTerm(name)
  {
    m_advection_speed.resize(1u);
    m_advection_speed[XX]= 1.;

    options().add_option("advection_speed",m_advection_speed).link_to(&m_advection_speed);
  }
  virtual ~LinearAdvection1D() {}

  virtual void compute_analytical_flux(ConvectiveTermPointData<1u,1u>& data, const RealVector1& unit_normal, RealVector1& flux, Real& wave_speed)
  {
    Real A = unit_normal[XX]*m_advection_speed[XX];
    flux = A*data.solution;
    wave_speed = std::abs(A);
  }

  virtual void compute_numerical_flux(ConvectiveTermPointData<1u,1u>& left, ConvectiveTermPointData<1u,1u>& right, const RealVector1& unit_normal, RealVector1& flux, Real& wave_speed)
  {
    Real A = m_advection_speed[XX]*unit_normal[XX];
    flux = 0.5 * A*(left.solution + right.solution) - 0.5 * std::abs(A)*(right.solution - left.solution);
    wave_speed = std::abs(A);
  }

private:
  std::vector<Real> m_advection_speed;
};

////////////////////////////////////////////////////////////////////////////////

} // scalar
} // SFDM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_SFDM_scalar_LinearAdvection1D_hpp
