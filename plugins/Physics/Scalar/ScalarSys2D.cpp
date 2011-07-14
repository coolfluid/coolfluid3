// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string/predicate.hpp>

#include "Common/CBuilder.hpp"

#include "Physics/Variables.hpp"

#include "ScalarSys2D.hpp"

namespace CF {
namespace Physics {
namespace Scalar {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Scalar::ScalarSys2D,
                           Physics::PhysModel,
                           LibScalar >
                           Builder_ScalarSys2D;

ScalarSys2D::ScalarSys2D( const std::string& name ) : Physics::PhysModel(name)
{
}

ScalarSys2D::~ScalarSys2D() {}

boost::shared_ptr< Physics::Variables > ScalarSys2D::create_variables( const std::string type, const std::string name )
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
} // CF
