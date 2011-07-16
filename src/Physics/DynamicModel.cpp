// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Physics/DynamicModel.hpp"
#include "Physics/DynamicVars.hpp"
#include "VariableManager.hpp"

namespace CF {
namespace Physics {

using namespace Common;

struct DynamicModel::Implementation
{
  Implementation() : m_type("DynamicModel")
  {
  }
  
  std::string m_type;   ///< name of the physics type
};

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Physics::DynamicModel,
                           Physics::PhysModel,
                           LibPhysics >
                           Builder_DynamicModel;
                           
DynamicModel::DynamicModel( const std::string& name ) :
  Physics::PhysModel(name),
  m_implementation(new Implementation())
{
}

DynamicModel::~DynamicModel()
{
}

Variables::Ptr DynamicModel::create_variables(const std::string type, const std::string name )
{
  if( type == DynamicVars::type_name() )
  {
    return create_component_ptr< DynamicVars >( name );
  }
  else
    throw ValueNotFound( FromHere(), "Unknown variable type \'" + type + "\'" );
}

////////////////////////////////////////////////////////////////////////////////

Uint DynamicModel::ndim() const
{
  return variable_manager().option("dimensions").value<Uint>();
}

////////////////////////////////////////////////////////////////////////////////

Uint DynamicModel::neqs() const
{
  return variable_manager().nb_dof();
}

////////////////////////////////////////////////////////////////////////////////

std::string DynamicModel::type() const
{
  return m_implementation->m_type;
}


////////////////////////////////////////////////////////////////////////////////

} // Physics
} // CF
