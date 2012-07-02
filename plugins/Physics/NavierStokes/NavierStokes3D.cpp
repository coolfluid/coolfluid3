// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string/predicate.hpp>

#include "common/Builder.hpp"
#include "common/OptionList.hpp"

#include "physics/Variables.hpp"

#include "NavierStokes3D.hpp"

namespace cf3 {
namespace physics {
namespace NavierStokes {

using namespace common;

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < NavierStokes::NavierStokes3D,
                           physics::PhysModel,
                           LibNavierStokes >
                           Builder_NavierStokes3D;

////////////////////////////////////////////////////////////////////////////////////////////

NavierStokes3D::NavierStokes3D( const std::string& name ) :
  physics::PhysModel(name),
  m_gamma(1.4),
  m_R(287.05)
{
  options().add("gamma",m_gamma)
      .description("Specific heat reatio")
      .link_to(&m_gamma);

  options().add("R",m_R)
      .description("Gas constant")
      .link_to(&m_R);
}

////////////////////////////////////////////////////////////////////////////////////////////

NavierStokes3D::~NavierStokes3D()
{
}

////////////////////////////////////////////////////////////////////////////////////////////

boost::shared_ptr< physics::Variables > NavierStokes3D::create_variables( const std::string type, const std::string name )
{
  boost::shared_ptr< physics::Variables > vars = boost::algorithm::contains( type, "." ) ?
        build_component_abstract_type< physics::Variables >( type, name ) :
        build_component_abstract_type< physics::Variables >( LibNavierStokes::library_namespace() + "." + type, name );

  add_component( vars );

  return vars;
}

////////////////////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // physics
} // cf3
