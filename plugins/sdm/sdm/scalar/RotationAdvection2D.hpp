// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_scalar_RotationAdvection2D_hpp
#define cf3_sdm_scalar_RotationAdvection2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include "sdm/ConvectiveTerm.hpp"
#include "sdm/scalar/LibScalar.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace scalar {

////////////////////////////////////////////////////////////////////////////////

/// @brief Convective SD term for linear advection with rotating speed
class sdm_scalar_API RotationAdvection2D : public ConvectiveTerm< PhysDataBase<1u,2u> >
{
public:
  static std::string type_name() { return "RotationAdvection2D"; }
  RotationAdvection2D(const std::string& name) : ConvectiveTerm< PhysData >(name)
  {
    m_omega = 1.;
    m_rotation_centre.resize(NDIM,0.);

    options().add("omega",m_omega)
        .description("Rotational velocity in [rad/s]")
        .link_to(&m_omega);

    options().add("rotation_centre",m_rotation_centre)
        .description("Centre of rotation")
        .link_to(&m_rotation_centre);
  }

  virtual ~RotationAdvection2D() {}

  virtual void compute_analytical_flux(PhysData& data, const RealVectorNDIM& unit_normal, RealVectorNEQS& flux, Real& wave_speed)
  {
    Real A = m_omega*(unit_normal[XX]*(data.coord[YY]-m_rotation_centre[YY])-unit_normal[YY]*(data.coord[XX]-m_rotation_centre[XX]));
    flux = A*data.solution;
    wave_speed = std::abs(A);
  }

  virtual void compute_numerical_flux(PhysData& left, PhysData& right, const RealVectorNDIM& unit_normal, RealVectorNEQS& flux, Real& wave_speed)
  {
    Real A = m_omega*(unit_normal[XX]*(left.coord[YY]-m_rotation_centre[YY])-unit_normal[YY]*(left.coord[XX]-m_rotation_centre[XX]));
    flux = 0.5 * A*(left.solution + right.solution) - 0.5 * std::abs(A)*(right.solution - left.solution);
    wave_speed = std::abs(A);
  }

private:

  Real m_omega;
  std::vector<Real> m_rotation_centre;
};

////////////////////////////////////////////////////////////////////////////////

} // scalar
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_scalar_RotationAdvection2D_hpp
