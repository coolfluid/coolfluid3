// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the variabless of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Signal.hpp"
#include "common/OptionT.hpp"
#include "common/TypeInfo.hpp"

#include "physics/PhysModel.hpp"

#include "math/VariableManager.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;

namespace cf3 {
namespace physics {

common::RegistTypeInfo<PhysModel, LibPhysics> PhysMod_type_info;
  
////////////////////////////////////////////////////////////////////////////////

PhysModel::PhysModel( const std::string& name ) :
  Component(name),
  m_variable_manager(*create_static_component<math::VariableManager>("VariableManager"))
{

  regist_signal( "create_variables" )
      .connect  ( boost::bind( &PhysModel::signal_create_variables, this, _1 ) )
      .signature( boost::bind( &PhysModel::signature_create_variables, this, _1))
      .description("Create Variables using this Physical Model")
      .pretty_name("Create Variables");

}

PhysModel::~PhysModel()
{
}

math::VariableManager& PhysModel::variable_manager()
{
  return m_variable_manager;
}

const math::VariableManager& PhysModel::variable_manager() const
{
  return m_variable_manager;
}

////////////////////////////////////////////////////////////////////////////////

void PhysModel::signal_create_variables( SignalArgs& args )
{
  SignalOptions options( args );

  std::string name = options.value<std::string>("name");
  std::string type = options.value<std::string>("type");

  create_variables( type, name );
}

////////////////////////////////////////////////////////////////////////////////

void PhysModel::signature_create_variables( SignalArgs& args )
{
  SignalOptions options( args );

  // name
  options.add("name", std::string() )
      .description("Name for created variables");

  // type
  options.add("type", std::string() )
      .description("Type for created variables");

}

////////////////////////////////////////////////////////////////////////////////

} // physics
} // cf3
