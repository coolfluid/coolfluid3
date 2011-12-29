// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_scalar_RotationAdvection2D_hpp
#define cf3_SFDM_scalar_RotationAdvection2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include "SFDM/ConvectiveTerm.hpp"
#include "SFDM/scalar/LibScalar.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {
namespace scalar {

////////////////////////////////////////////////////////////////////////////////

class SFDM_scalar_API RotationAdvection2D : public ConvectiveTerm<1u,2u>
{
public:
  static std::string type_name() { return "RotationAdvection2D"; }
  RotationAdvection2D(const std::string& name) : ConvectiveTerm(name)
  {
    m_omega = 1.;
    m_rotation_centre.resize(2,0.);

    options().add_option("omega",m_omega)
        .description("Rotational velocity in [rad/s]")
        .link_to(&m_omega);

    options().add_option("rotation_centre",m_rotation_centre)
        .description("Centre of rotation")
        .link_to(&m_rotation_centre);
  }

  virtual ~RotationAdvection2D() {}

  virtual void initialize()
  {
    ConvectiveTerm::initialize();
    flx_pt_coordinates = shared_caches().get_cache< FluxPointCoordinates<2u> >();
    flx_pt_coordinates->options().configure_option("space",solution_field().space());
  }

  virtual void set_entities(const mesh::Entities& entities)
  {
    ConvectiveTerm::set_entities(entities);
    flx_pt_coordinates->cache(m_entities);
  }

  virtual void set_element(const Uint elem_idx)
  {
    ConvectiveTerm::set_element(elem_idx);
    flx_pt_coordinates->get().compute_element(m_elem_idx);
  }

  virtual void compute_analytical_flux(const RealVector2& unit_normal)
  {
    Real A = m_omega*(unit_normal[XX]*(flx_pt_coordinates->get()[flx_pt][YY]-m_rotation_centre[YY])-unit_normal[YY]*(flx_pt_coordinates->get()[flx_pt][XX]-m_rotation_centre[XX]));
    flx_pt_flux[flx_pt] = A*flx_pt_solution->get()[flx_pt];
    flx_pt_wave_speed[flx_pt][0] = std::abs(A);
  }

  virtual void compute_numerical_flux(const RealVector2& unit_normal)
  {
    RealVector1& left  = flx_pt_solution->get()[flx_pt];
    RealVector1& right = flx_pt_neighbour_solution->get()[neighbour_flx_pt];
    Real A = m_omega*(unit_normal[XX]*(flx_pt_coordinates->get()[flx_pt][YY]-m_rotation_centre[YY])-unit_normal[YY]*(flx_pt_coordinates->get()[flx_pt][XX]-m_rotation_centre[XX]));
    flx_pt_flux[flx_pt] = 0.5 * A*(left + right) - 0.5 * std::abs(A)*(right - left);
    flx_pt_wave_speed[flx_pt][0] = std::abs(A);
  }

  virtual void unset_element()
  {
    ConvectiveTerm::unset_element();
    flx_pt_coordinates->get().unlock();
  }

private:

  Real m_omega;
  std::vector<Real> m_rotation_centre;

  Handle< CacheT< FluxPointCoordinates<2u> > > flx_pt_coordinates;
};

////////////////////////////////////////////////////////////////////////////////

} // scalar
} // SFDM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_SFDM_scalar_RotationAdvection2D_hpp
