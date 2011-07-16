// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Physics/PhysModel.hpp"
#include "Physics/VariableManager.hpp"

namespace CF {
namespace Physics {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

PhysModel::PhysModel( const std::string& name ) :
  Component(name),
  m_variable_manager(create_static_component<VariableManager>("VariableManager"))
{
}

PhysModel::~PhysModel()
{
}

VariableManager& PhysModel::variable_manager()
{
  return m_variable_manager;
}

const VariableManager& PhysModel::variable_manager() const
{
  return m_variable_manager;
}

////////////////////////////////////////////////////////////////////////////////

} // Physics
} // CF
