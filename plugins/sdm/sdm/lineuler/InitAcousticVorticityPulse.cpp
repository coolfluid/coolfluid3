// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

//#include <boost/math/special_functions/bessel.hpp>

#include "common/Log.hpp"
#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionComponent.hpp"

#include "math/Checks.hpp"

#include "mesh/Elements.hpp"
#include "mesh/Region.hpp"
#include "mesh/Field.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Space.hpp"

#include "sdm/lineuler/InitAcousticVorticityPulse.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace lineuler {

  using namespace common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < InitAcousticVorticityPulse, common::Action, sdm::lineuler::LibLinEuler> InitAcousticVorticityPulse_Builder;

//////////////////////////////////////////////////////////////////////////////

InitAcousticVorticityPulse::InitAcousticVorticityPulse( const std::string& name )
  : common::Action(name)
{

  properties()["brief"] = std::string("Initialize a field with a constant value");
  std::string desc;
  desc = "  Usage: InitAcousticVorticityPulse constant \n";
  properties()["description"] = desc;

  options().add("field", m_field)
      .description("Field to initialize")
      .pretty_name("Field")
      .link_to(&m_field)
      .mark_basic();

  options().add("time", 0.).description("time after pulse").mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void InitAcousticVorticityPulse::execute()
{
  RealVector2 coord;
  Real time = options().value<Real>("time");

  cf3_assert(m_field);
  cf3_assert(m_field->coordinates().row_size()>=DIM_2D);
  cf3_assert(m_field->row_size()==4);
  for (Uint i=0; i<m_field->size(); ++i)
  {
    cf3_assert(i<m_field->coordinates().size());
    coord[XX] = m_field->coordinates()[i][XX];
    coord[YY] = m_field->coordinates()[i][YY];

//    std::cout << i << ": " << coord.transpose() << "   p=" << compute_pressure(coord,time) << std::endl;

    RealVector velocity = compute_velocity(coord,time);
    Real pressure = compute_pressure(coord,time);
    Real density = compute_density(pressure,coord,time);

    m_field->array()[i][0] = density;
    m_field->array()[i][1] = velocity[XX];
    m_field->array()[i][2] = velocity[YY];
    m_field->array()[i][3] = pressure;
  }

}

//////////////////////////////////////////////////////////////////////////////

InitAcousticVorticityPulse::Data::Data()
{
    u0 = 0.5;
    alpha1 = std::log(2.)/9.;
    alpha2 = std::log(2.)/25.;
    eta = 0;
    s0 = 0;
    s1 = 1;
    while (std::exp(-s1*s1/(4.*alpha1)) > 1e-60)
    {
      s1+=1;
    }
}

Real InitAcousticVorticityPulse::eta(const RealVector& coord, const Real& t) const
{
  return std::sqrt( (coord[XX]-m_data.u0*t)*(coord[XX]-m_data.u0*t) + coord[YY]*coord[YY]);
}

/// Actual function to be integrated
Real InitAcousticVorticityPulse::PressureIntegrand::operator()(Real lambda) const
{
  return std::exp(-lambda*lambda/(4.*m_data.alpha1))*std::cos(lambda*m_data.time)*j0(lambda*m_data.eta)*lambda;
}

/// Actual function to be integrated
Real InitAcousticVorticityPulse::VelocityIntegrand::operator()(Real lambda) const
{
  return std::exp(-lambda*lambda/(4.*m_data.alpha1))*std::sin(lambda*m_data.time)*j1(lambda*m_data.eta)*lambda;
}


RealVector InitAcousticVorticityPulse::compute_velocity(const RealVector& coord, const Real& t)
{
  m_data.time = t;
  m_data.eta = eta(coord,t);
  Real x_vort = (coord[XX]-67.) - m_data.u0*t;
  Real y_vort = coord[YY];

  RealVector u(2);
  Real integral = integrate( VelocityIntegrand(m_data), m_data.s0,m_data.s1);
  u[XX] = (coord[XX]-m_data.u0*t)/(2.*m_data.alpha1*m_data.eta) * integral + 0.04*y_vort*std::exp(-m_data.alpha2*(x_vort*x_vort+y_vort*y_vort));
  u[YY] = (coord[YY]            )/(2.*m_data.alpha1*m_data.eta) * integral - 0.04*x_vort*std::exp(-m_data.alpha2*(x_vort*x_vort+y_vort*y_vort));
  for (Uint d=0; d<2; ++d)
  {
    if (std::abs(u[d])<1e-12)
      u[d];
    if (math::Checks::is_nan(u[d]))
      u[d]=0.;
  }
  return u;
}

Real InitAcousticVorticityPulse::compute_pressure(const RealVector& coord, const Real& t)
{
  m_data.time = t;
  m_data.eta = eta(coord,t);
  const Real x_vort = (coord[XX]-67.) - m_data.u0*t;
  const Real y_vort = coord[YY];
  const Real p = 1./(2.*m_data.alpha1) * integrate( PressureIntegrand(m_data), m_data.s0,m_data.s1);
  if (std::abs(p)<1e-12)
    return 0.;
  if (math::Checks::is_nan(p))
    return 0.;
  return p;
}

Real InitAcousticVorticityPulse::compute_density(const Real& pressure, const RealVector& coord, const Real& t)
{
  m_data.time = t;
  m_data.eta = eta(coord,t);
  Real x_vort = (coord[XX]-67.) - m_data.u0*t;
  Real y_vort = coord[YY];
  Real rho = pressure + 0.1*std::exp(-m_data.alpha2*(x_vort*x_vort+y_vort*y_vort));
  if (std::abs(rho)<1e-12)
    return 0.;
  if (math::Checks::is_nan(rho))
    return 0.;
  return rho;
}


//////////////////////////////////////////////////////////////////////////////

} // LinEuler
} // physics
} // cf3
