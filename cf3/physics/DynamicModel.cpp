// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/Tags.hpp"

#include "physics/DynamicModel.hpp"
#include "physics/DynamicVars.hpp"

#include "math/VariableManager.hpp"
#include "math/VariablesDescriptor.hpp"

namespace cf3 {
namespace physics {

using namespace common;
using namespace math;

struct DynamicModel::Implementation
{
  Implementation(Component& component) : m_component(component), m_type("DynamicModel"), m_updating(false)
  {
    m_component.options().add(common::Tags::dimension(), 0u)
      .pretty_name("Dimensions")
      .description("Dimensions for the problem")
      .attach_trigger(boost::bind(&Implementation::trigger_dimensions, this));
  }

  void trigger_dimensions()
  {
    if(m_updating)
      return;

    m_updating = true;
    m_dimensions = m_component.options().option(common::Tags::dimension()).value<Uint>();
    m_component.configure_option_recursively(common::Tags::dimension(), m_dimensions);
    m_updating = false;
  }

  Component& m_component;
  std::string m_type;   ///< name of the physics type
  Uint m_dimensions;
  bool m_updating;
};

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < physics::DynamicModel,
                           physics::PhysModel,
                           LibPhysics >
                           Builder_DynamicModel;

DynamicModel::DynamicModel( const std::string& name ) :
  physics::PhysModel(name),
  m_implementation(new Implementation(*this))
{
}

DynamicModel::~DynamicModel()
{
}

boost::shared_ptr< Variables > DynamicModel::create_variables(const std::string type, const std::string name )
{
  if( type == DynamicVars::type_name() )
  {
    boost::shared_ptr<Variables> vars = allocate_component<DynamicVars>(name);
    add_component(vars);
    return vars;
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

} // physics
} // cf3
