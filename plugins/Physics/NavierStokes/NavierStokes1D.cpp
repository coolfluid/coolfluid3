// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string/predicate.hpp>

#include "common/Builder.hpp"
#include "common/OptionT.hpp"

#include "Physics/Variables.hpp"

#include "NavierStokes1D.hpp"

namespace cf3 {
namespace Physics {
namespace NavierStokes {

using namespace common;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < NavierStokes::NavierStokes1D,
                           Physics::PhysModel,
                           LibNavierStokes >
                           Builder_NavierStokes1D;

////////////////////////////////////////////////////////////////////////////////////////////

NavierStokes1D::NavierStokes1D( const std::string& name ) :
  Physics::PhysModel(name),
  m_gamma(1.4),
  m_R(287.05)
{
  options().add_option< OptionT<Real> >("gamma",m_gamma)
      ->description("Specific heat reatio")
      ->link_to(&m_gamma);

  options().add_option< OptionT<Real> >("R",m_R)
      ->description("Gas constant")
      ->link_to(&m_R);
}

////////////////////////////////////////////////////////////////////////////////////////////

NavierStokes1D::~NavierStokes1D()
{
}

////////////////////////////////////////////////////////////////////////////////////////////

boost::shared_ptr< Physics::Variables > NavierStokes1D::create_variables( const std::string type, const std::string name )
{
  Physics::Variables::Ptr vars = boost::algorithm::contains( type, "." ) ?
        build_component_abstract_type< Physics::Variables >( type, name ) :
        build_component_abstract_type< Physics::Variables >( LibNavierStokes::library_namespace() + "." + type, name );

  add_component( vars );

  return vars;
}

////////////////////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // Physics
} // cf3
