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

#include "mesh/Elements.hpp"
#include "mesh/Region.hpp"
#include "mesh/Field.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Space.hpp"

#include "sdm/lineuler/InitAcousticPulse.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace lineuler {

  using namespace common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < InitAcousticPulse, common::Action, sdm::lineuler::LibLinEuler> InitAcousticPulse_Builder;

//////////////////////////////////////////////////////////////////////////////

InitAcousticPulse::InitAcousticPulse( const std::string& name )
  : common::Action(name)
{

  properties()["brief"] = std::string("Initialize a field with a constant value");
  std::string desc;
  desc = "  Usage: InitAcousticPulse constant \n";
  properties()["description"] = desc;

  options().add("field", m_field)
      .description("Field to initialize")
      .pretty_name("Field")
      .link_to(&m_field)
      .mark_basic();

  options().add("time", 0.).description("time after pulse").mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void InitAcousticPulse::execute()
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

// before:        exp( -log(2.)/9.  *  (x^2+y^2) )
// now:     0.001*exp( -1/(0.05^2)  *  (x^2+y^2) )',

// now: alpha1 = 1/(b^2)
//        or b = 1/sqrt(alpha1)
InitAcousticPulse::Data::Data()
{
    b = 0.05;
    c0 = std::sqrt(1.4);
    u0 = 0.0;
    alpha1 = 1./(b*b);
    eta = 0;
    s0 = 0;
    s1 = 1;
    while (std::exp(-s1*s1/(4.*alpha1)) > 1e-60)
    {
      s1+=1;
    }
}

Real InitAcousticPulse::eta(const RealVector& coord, const Real& t) const
{
  return std::sqrt( (coord[XX]-m_data.u0*t)*(coord[XX]-m_data.u0*t) + coord[YY]*coord[YY]);
}

/// Actual function to be integrated
Real InitAcousticPulse::PressureIntegrand::operator()(Real lambda) const
{
  return std::exp(-lambda*lambda*m_data.b*m_data.b/(2*2))*std::cos(lambda*m_data.time*m_data.c0)*j0(lambda*m_data.eta)*lambda;
}

/// Actual function to be integrated
Real InitAcousticPulse::VelocityIntegrand::operator()(Real lambda) const
{
  return std::exp(-lambda*lambda/(4.*m_data.alpha1))*std::sin(lambda*m_data.time*m_data.c0)*j1(lambda*m_data.eta)*lambda;
}


RealVector InitAcousticPulse::compute_velocity(const RealVector& coord, const Real& t)
{
  m_data.time = t;
  m_data.eta = eta(coord,t);
  RealVector u(2);
  Real integral = integrate( VelocityIntegrand(m_data), m_data.s0,m_data.s1);
  u[XX] = m_data.c0*(coord[XX]-m_data.u0*t)/(2.*m_data.alpha1*m_data.eta) * integral;
  u[YY] = m_data.c0*(coord[YY]            )/(2.*m_data.alpha1*m_data.eta) * integral;
  return 0.001 * u;
}

Real InitAcousticPulse::compute_pressure(const RealVector& coord, const Real& t)
{
  m_data.time = t;
  m_data.eta = eta(coord,t);
  return 0.001 * m_data.c0*m_data.c0*m_data.b*m_data.b/2 * integrate( PressureIntegrand(m_data), m_data.s0,m_data.s1);
}

Real InitAcousticPulse::compute_density(const Real& pressure, const RealVector& coord, const Real& t)
{
  return pressure/(m_data.c0*m_data.c0);
}


//////////////////////////////////////////////////////////////////////////////

} // LinEuler
} // physics
} // cf3
