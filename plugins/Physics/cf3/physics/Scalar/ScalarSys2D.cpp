// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string/predicate.hpp>

#include "cf3/common/Builder.hpp"

#include "cf3/physics/Variables.hpp"

#include "cf3/physics/Scalar/ScalarSys2D.hpp"

namespace cf3 {
namespace physics {
namespace Scalar {

using namespace common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Scalar::ScalarSys2D,
                           physics::PhysModel,
                           LibScalar >
                           Builder_ScalarSys2D;

ScalarSys2D::ScalarSys2D( const std::string& name ) : physics::PhysModel(name)
{
}

ScalarSys2D::~ScalarSys2D() {}

boost::shared_ptr< physics::Variables > ScalarSys2D::create_variables( const std::string type, const std::string name )
{
  boost::shared_ptr< physics::Variables > vars = boost::algorithm::contains( type, "." ) ?
        build_component_abstract_type< physics::Variables >( type, name ) :
        build_component_abstract_type< physics::Variables >( LibScalar::library_namespace() + "." + type, name );

  add_component( vars );

  return vars;
}

////////////////////////////////////////////////////////////////////////////////

} // Scalar
} // physics
} // cf3
