// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string/predicate.hpp>

#include "Common/CBuilder.hpp"

#include "Physics/Variables.hpp"

#include "LinEuler/LinEuler2D.hpp"

namespace CF {
namespace Physics {
namespace LinEuler {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < LinEuler::LinEuler2D,
                           Physics::PhysModel,
                           LibLinEuler >
                           Builder_LinEuler2D;

LinEuler2D::LinEuler2D( const std::string& name ) : Physics::PhysModel(name)
{
}

LinEuler2D::~LinEuler2D()
{
}

boost::shared_ptr< Physics::Variables > LinEuler2D::create_variables( const std::string type, const std::string name )
{
  Physics::Variables::Ptr vars = boost::algorithm::contains( type, "." ) ?
        build_component_abstract_type< Physics::Variables >( type, name ) :
        build_component_abstract_type< Physics::Variables >( LibLinEuler::library_namespace() + "." + type, name );

  add_component( vars );

  return vars;
}

////////////////////////////////////////////////////////////////////////////////

} // LinEuler
} // Physics
} // CF
