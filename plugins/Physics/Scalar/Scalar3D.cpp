// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Scalar3D.hpp"

namespace CF {
namespace Physics {
namespace Scalar {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

Scalar3D::Scalar3D( const std::string& name ) : Physics::PhysModel(name)
{
}

Scalar3D::~Scalar3D()
{
}

boost::shared_ptr< Physics::Variables > Scalar3D::create_variables( const std::string type )
{
  Physics::Variables::Ptr vars = boost::algorithm::contains( type, "." ) ?
        build_component_abstract_type< Physics::Variables >( type ) :
        build_component_abstract_type< Physics::Variables >( LibScalar::library_namespace() + "." + type );

  add_component( vars );

  return vars;
}
////////////////////////////////////////////////////////////////////////////////

} // Scalar
} // Physics
} // CF
