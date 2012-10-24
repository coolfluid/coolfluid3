// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/OptionArray.hpp"

#include "physics/Variables.hpp"

#include "LinEuler/LinEuler2D.hpp"

namespace cf3 {
namespace physics {
namespace LinEuler {

using namespace common;

////////////////////////////////////////////////////////////////////////////////

LinEuler2D::Properties::Properties()
{
  gamma = 1.0; /// @todo this value is set for atest-rdm-rklineuler

  rho0 = 1.;
  u0 = (LinEuler2D::GeoV() << 0.5 , 0.).finished(); /// @todo this value is set for atest-rdm-rklineuler
  P0 = 1.;

  inv_rho0 = 1./rho0;
  c=sqrt(gamma*P0*inv_rho0);
  inv_c = 1./c;
}

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < LinEuler::LinEuler2D,
                           physics::PhysModel,
                           LibLinEuler >
                           Builder_LinEuler2D;

LinEuler2D::LinEuler2D( const std::string& name ) :
  physics::PhysModel(name),
  m_gamma(1.), /// @todo this value is set for atest-rdm-rklineuler
  m_rho0(1.),
  m_u0( (LinEuler2D::GeoV() << 0.5 , 0.).finished() ),  /// @todo this value is set for atest-rdm-rklineuler
  m_P0(1.)
{
  options().add("gamma",m_gamma)
      .description("Specific heat reatio")
      .mark_basic()
      .link_to(&m_gamma);

  options().add("rho0",m_rho0)
      .description("Uniform mean density")
      .mark_basic()
      .link_to(&m_rho0);

  std::vector<Real> U0(m_u0.size());
  U0[XX] = m_u0[XX];
  U0[YY] = m_u0[YY];
  options().add("U0",U0)
      .description("Uniform mean velocity")
      .mark_basic()
      .attach_trigger( boost::bind( &LinEuler2D::config_mean_velocity, this) );

  options().add("P0",m_P0)
      .mark_basic()
      .description("Uniform mean pressure")
      .link_to(&m_P0);
}

void LinEuler2D::config_mean_velocity()
{
  std::vector<Real> U0 = options().value< std::vector<Real> >("U0");
  m_u0[XX] = U0[XX];
  m_u0[YY] = U0[YY];
}

void LinEuler2D::set_constants(LinEuler2D::Properties& props)
{
  props.gamma = m_gamma;
  props.rho0 = m_rho0;
  props.u0 = m_u0;
  props.P0 = m_P0;

  props.inv_rho0 = 1./props.rho0;
  props.c=sqrt(props.gamma*props.P0*props.inv_rho0);
  props.inv_c = 1./props.c;
}


LinEuler2D::~LinEuler2D()
{
}

boost::shared_ptr< physics::Variables > LinEuler2D::create_variables( const std::string type, const std::string name )
{
  boost::shared_ptr< physics::Variables > vars = boost::algorithm::contains( type, "." ) ?
        build_component_abstract_type< physics::Variables >( type, name ) :
        build_component_abstract_type< physics::Variables >( LibLinEuler::library_namespace() + "." + type, name );

  add_component( vars );

  return vars;
}

////////////////////////////////////////////////////////////////////////////////

} // LinEuler
} // physics
} // cf3
