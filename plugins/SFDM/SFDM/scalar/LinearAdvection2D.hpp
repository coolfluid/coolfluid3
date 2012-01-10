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

  virtual void compute_analytical_flux(ConvectiveTermPointData<NEQS,NDIM>& data, const Eigen::Matrix<Real,NDIM,1>& unit_normal, Eigen::Matrix<Real,NEQS,1>& flux, Real& wave_speed)
  {
    Real A = (unit_normal[XX]*m_advection_speed[XX]+unit_normal[YY]*m_advection_speed[YY]);
    flux = A*data.solution;
    wave_speed = std::abs(A);
  }

  virtual void compute_numerical_flux(ConvectiveTermPointData<NEQS,NDIM>& left, ConvectiveTermPointData<NEQS,NDIM>& right, const Eigen::Matrix<Real,NDIM,1>& unit_normal, Eigen::Matrix<Real,NEQS,1>& flux, Real& wave_speed)
  {
    Real A = (m_advection_speed[XX]*unit_normal[XX]+m_advection_speed[YY]*unit_normal[YY]);
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

#endif // cf3_SFDM_scalar_LinearAdvection2D_hpp
