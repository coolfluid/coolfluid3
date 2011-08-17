// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/FindComponents.hpp"
#include "Common/OptionT.hpp"
#include "Common/Tags.hpp"

#include "Physics/DynamicModel.hpp"
#include "Physics/DynamicVars.hpp"

#include "Math/VariableManager.hpp"
#include "Math/VariablesDescriptor.hpp"

namespace CF {
namespace Physics {

using namespace Common;
using namespace Math;

struct DynamicModel::Implementation
{
  Implementation(Component& component) : m_component(component), m_type("DynamicModel"), m_updating(false)
  {
    m_component.options().add_option< OptionT<Uint> >(Common::Tags::dimension(), 0u)
      ->pretty_name("Dimensions")
      ->description("Dimensions for the problem")
      ->attach_trigger(boost::bind(&Implementation::trigger_dimensions, this));
  }
  
  void trigger_dimensions()
  {
    if(m_updating)
      return;
    
    m_updating = true;
    m_dimensions = m_component.option(Common::Tags::dimension()).value<Uint>();
    m_component.configure_option_recursively(Common::Tags::dimension(), m_dimensions);
    m_updating = false;
  }
  
  Component& m_component;
  std::string m_type;   ///< name of the physics type
  Uint m_dimensions;
  bool m_updating;
};

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Physics::DynamicModel,
                           Physics::PhysModel,
                           LibPhysics >
                           Builder_DynamicModel;
                           
DynamicModel::DynamicModel( const std::string& name ) :
  Physics::PhysModel(name),
  m_implementation(new Implementation(*this))
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
  return m_implementation->m_dimensions;
}

////////////////////////////////////////////////////////////////////////////////

Uint DynamicModel::neqs() const
{
  Uint nb_eqs = 0;
  boost_foreach(const VariablesDescriptor& var_desc, find_components<VariablesDescriptor>(variable_manager()))
  {
    nb_eqs += var_desc.size();
  }
  
  return nb_eqs;
}

////////////////////////////////////////////////////////////////////////////////

std::string DynamicModel::type() const
{
  return m_implementation->m_type;
}


////////////////////////////////////////////////////////////////////////////////

} // Physics
} // CF
