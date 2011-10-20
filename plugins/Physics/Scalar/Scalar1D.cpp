// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string/predicate.hpp>

#include "common/CBuilder.hpp"
#include "common/OptionT.hpp"

#include "Physics/Variables.hpp"

#include "Scalar1D.hpp"

namespace cf3 {
namespace Physics {
namespace Scalar {

using namespace common;

////////////////////////////////////////////////////////////////////////////////

Scalar1D::Properties::Properties()
{
  v = 1.0;
  mu = 1.0;
}

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Scalar::Scalar1D,
                           Physics::PhysModel,
                           LibScalar >
                           Builder_Scalar1D;

////////////////////////////////////////////////////////////////////////////////

Scalar1D::Scalar1D( const std::string& name ) :
  Physics::PhysModel(name),
  m_v (1.0),
  m_mu (1.0)
{
  options().add_option< OptionT<Real> >("v",m_v)
      ->description("Advection Speed")
      ->link_to(&m_v);

  options().add_option< OptionT<Real> >("mu",m_mu)
      ->description("Diffusion Coefficient")
      ->link_to(&m_mu);
}

////////////////////////////////////////////////////////////////////////////////

Scalar1D::~Scalar1D() {}

////////////////////////////////////////////////////////////////////////////////

void Scalar1D::set_constants(Scalar1D::Properties& props)
{
  props.v = m_v;
  props.mu = m_mu;
}

////////////////////////////////////////////////////////////////////////////////

boost::shared_ptr< Physics::Variables > Scalar1D::create_variables( const std::string type, const std::string name )
{
  Physics::Variables::Ptr vars = boost::algorithm::contains( type, "." ) ?
        build_component_abstract_type< Physics::Variables >( type, name ) :
        build_component_abstract_type< Physics::Variables >( LibScalar::library_namespace() + "." + type, name );

  add_component( vars );

  return vars;
}

////////////////////////////////////////////////////////////////////////////////

} // Scalar
} // Physics
} // cf3
