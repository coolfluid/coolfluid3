// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string/predicate.hpp>

#include "Common/CBuilder.hpp"

#include "Physics/Variables.hpp"

#include "Scalar3D.hpp"

namespace cf3 {
namespace Physics {
namespace Scalar {

using namespace common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Scalar::Scalar3D,
                           Physics::PhysModel,
                           LibScalar >
                           Builder_Scalar3D;

Scalar3D::Scalar3D( const std::string& name ) : Physics::PhysModel(name)
{
}

Scalar3D::~Scalar3D()
{
}

boost::shared_ptr< Physics::Variables > Scalar3D::create_variables( const std::string type, const std::string name )
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
