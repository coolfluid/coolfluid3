// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the variabless of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Signal.hpp"
#include "Common/OptionT.hpp"

#include "Physics/PhysModel.hpp"

#include "Math/VariableManager.hpp"

namespace CF {
namespace Physics {

using namespace Common;
using namespace CF::Common::XML;

////////////////////////////////////////////////////////////////////////////////

PhysModel::PhysModel( const std::string& name ) :
  Component(name),
  m_variable_manager(create_static_component<Math::VariableManager>("VariableManager"))
{

  regist_signal( "create_variables" )
      ->connect  ( boost::bind( &PhysModel::signal_create_variables, this, _1 ) )
      ->signature( boost::bind( &PhysModel::signature_create_variables, this, _1))
      ->description("Create Variables using this Physical Model")
      ->pretty_name("Create Variables");

}

PhysModel::~PhysModel()
{
}

Math::VariableManager& PhysModel::variable_manager()
{
  return m_variable_manager;
}

const Math::VariableManager& PhysModel::variable_manager() const
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
  options.add_option< OptionT<std::string> >("name", std::string() )
      ->description("Name for created variables");

  // type
  options.add_option< OptionT<std::string> >("type", std::string() )
      ->description("Type for created variables");

}

////////////////////////////////////////////////////////////////////////////////

} // Physics
} // CF
