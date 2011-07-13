// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Physics/DynamicModel.hpp"

#include "Physics/DynamicVars.hpp"

namespace CF {
namespace Physics {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

DynamicModel::DynamicModel( const std::string& name ) : Physics::PhysModel(name)
{
}

DynamicModel::~DynamicModel()
{
}

Variables::Ptr DynamicModel::create_variables(const std::string type)
{
  if( type == DynamicVars::type_name() )
  {
    return create_component_ptr< DynamicVars >( DynamicVars::type_name() );
  }
  else
    throw ValueNotFound( FromHere(), "Unknown variable type \'" + type + "\'" );
}

////////////////////////////////////////////////////////////////////////////////

} // Physics
} // CF
