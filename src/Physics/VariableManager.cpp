// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>

#include "Common/Foreach.hpp"
#include "Common/OptionT.hpp"

#include "Physics/VariableManager.hpp"

namespace CF {
namespace Physics {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////


struct VariableManager::Implementation
{
  Implementation(Component& component) :
    m_component(component),
    m_dim(0u),
    m_nbdofs(0u)
  {
    m_component.options().add_option<OptionT <Uint> >("dimensions", 0)
      ->set_pretty_name("Dimensions")
      ->set_description("Dimensionality of the problem, i.e. the number of components for the spatial coordinates")
      ->mark_basic()
      ->link_to(&m_dim)
      ->attach_trigger(boost::bind(&Implementation::trigger_dimensions, this));
  }
  
  void trigger_dimensions()
  {
    // Re-initialize, in case variables were registered without the dimension being known
    m_nbdofs = 0;

    // Calculate offset and nb_dofs
    boost_foreach(const std::string& eqn_var_name, m_state_variables)
    {
      const Uint var_size = m_variable_types[eqn_var_name] == SCALAR ? 1 : m_dim;
      m_variable_offsets[eqn_var_name] = m_nbdofs;
      m_nbdofs += var_size;
    }
  }
  
  std::string field_property_name(std::string var_name)
  {
    boost::to_lower(var_name);
    return var_name + "_field_name";
  }
  
  std::string variable_property_name(std::string var_name)
  {
    boost::to_lower(var_name);
    return var_name + "_variable_name";
  }
  
  bool is_state_variable(const std::string& var_name) const
  {
    return m_variable_offsets.count(var_name);
  }

  Uint offset(const std::string& var_name) const
  {
    std::map<std::string, Uint>::const_iterator it = m_variable_offsets.find(var_name);
    if(it == m_variable_offsets.end())
      throw Common::ValueNotFound(FromHere(), "Offset for variable " + var_name + " not found. Not an equation variable?");

    return it->second;
  }


  void register_variable(const std::string& name, std::string& symbol, std::string& field_name, const VariableTypesT var_type, const bool is_state)
  {
    if(m_variable_types.insert(std::make_pair(name, var_type)).second) // Check if the variable was registered
    {
      if(is_state) // Add to the state variables, if the variable belongs to the state
      {
        m_state_variables.push_back(name);
        m_variable_offsets[name] = m_nbdofs;
        m_nbdofs += var_type == SCALAR ? 1 : m_dim;
      }
      
      // Add options for changing the variable name and field name
      m_component.options().add_option< OptionT<std::string> >(field_property_name(name), field_name)
        ->set_pretty_name(name + std::string(" Field Name"))
        ->set_description("Field name for variable " + name)
        ->link_to(&field_name);

      m_component.options().add_option< OptionT<std::string> >(variable_property_name(name), symbol)
        ->set_pretty_name(name + std::string(" Variable Name"))
        ->set_description("Variable name for variable " + name)
        ->link_to(&symbol);
    }
    else
    {
      if(is_state && !std::count(m_state_variables.begin(), m_state_variables.end(), name))
      {
        m_state_variables.push_back(name);
        m_variable_offsets[name] = m_nbdofs;
        m_nbdofs += var_type == SCALAR ? 1 : m_dim;
      }
      
      m_component.option(field_property_name(name)).link_to(&field_name);
      m_component.option(variable_property_name(name)).link_to(&symbol);
    }
  }
/*
  void create_fields()
  {
    CMesh& mesh = mesh_checked();
    cf_assert(m_dim);

    

    // Re-initialize, in case variables were registered without the dimension being known
    m_nbdofs = 0;

    // Calculate offset and nb_dofs
    boost_foreach(const std::string& eqn_var_name, m_state_variables)
    {
      const Uint var_size = m_variable_types[eqn_var_name] == SCALAR ? 1 : m_dim;
      m_variable_offsets[eqn_var_name] = m_nbdofs;
      m_nbdofs += var_size;
    }

    typedef std::map<std::string, VarTypesT > FieldsT;
    FieldsT fields;

    // Collect the unique field and variable names
    for(VarTypesT::const_iterator it = m_variable_types.begin(); it != m_variable_types.end(); ++it)
    {
      const std::string internal_name = it->first;
      fields[m_field_names[internal_name]][m_variable_names[internal_name]] = it->second;
    }

    for(FieldsT::const_iterator fd_it = fields.begin(); fd_it != fields.end(); ++fd_it)
    {
      const std::string fd_name = fd_it->first;

      // Check if the field exists
      Component::ConstPtr existing_component = mesh.get_child_ptr(fd_name);
      CField::ConstPtr existing_field;
      if(existing_component)
        existing_field = existing_component->as_ptr<CField>();
      if(existing_component && !existing_field)
        throw ValueExists(FromHere(), "A component with the name of field " + fd_name + " exists. Field can not be created.");

      std::vector<std::string> fd_var_names;
      std::vector<Mesh::CField::VarType> fd_var_types;

      for(VarTypesT::const_iterator var_it = fd_it->second.begin(); var_it != fd_it->second.end(); ++var_it)
      {
        fd_var_names.push_back(var_it->first);
        fd_var_types.push_back(static_cast<Mesh::CField::VarType>(var_it->second == SCALAR ? 1 : m_dim));

        if(existing_field) // If the field exists, check if it is compatible
        {
          if(!existing_field->has_variable(fd_var_names.back()) // Variable doesn't exist
            || (existing_field->has_variable(fd_var_names.back()) && existing_field->var_type(fd_var_names.back()) != fd_var_types.back())) // Variable exists but size is incorrect
            throw Common::ValueExists(FromHere(), "Field with name " + fd_name + " exists, but is incompatible with the requested solution.");
        }
      }

      if(!existing_field)
        mesh.create_field(fd_name, CField::Basis::POINT_BASED, fd_var_names, fd_var_types);
    }
  }
  */

  void field_specification(std::map< std::string, std::string >& fields)
  {
    for(VarTypesT::const_iterator it = m_variable_types.begin(); it != m_variable_types.end(); ++it)
    {
      const std::string internal_name = it->first;
      const std::string variable_name = m_component.option(variable_property_name(internal_name)).value_str();
      const std::string field_name = m_component.option(field_property_name(internal_name)).value_str();
      
      const std::string& old_spec = fields[field_name];
      
      // Specification for the variable: variable_name[dimension of variable], inserting a , separator if needed
      std::stringstream var_spec;
      var_spec << (old_spec.empty() ? "" : ",") << variable_name << "[" << (it->second == SCALAR ? 1 : m_dim) << "]";
      
      // Append the specification for this variable to the list for the field
      fields[field_name] += var_spec.str();
    }
  }

  Component& m_component;

  /// dimensionality of physics
  Uint m_dim;

  /// number of degrees of freedom
  Uint m_nbdofs;
  
  /// Offset of each equation variable, i.e. in V (vector of u and v) and p, V has offset 0, and p has offset 2 when the order is uvp in the global system
  std::map<std::string, Uint> m_variable_offsets;
  
  /// Ordered list of the equation variables
  std::vector<std::string> m_state_variables;
  
  /// Type of each variable
  typedef std::map<std::string, VariableTypesT> VarTypesT;
  VarTypesT m_variable_types;
};

////////////////////////////////////////////////////////////////////////////////

VariableManager::VariableManager( const std::string& name ) :
  Component(name),
  m_implementation(new Implementation(*this))
{
}

VariableManager::~VariableManager()
{
}

void VariableManager::register_variable(const std::string& var_name, std::string& symbol, std::string& field_name, const VariableManager::VariableTypesT var_type, const bool is_state)
{
  m_implementation->register_variable(var_name, symbol, field_name, var_type, is_state);
}

bool VariableManager::is_state_variable(const std::string& var_name) const
{
  return m_implementation->is_state_variable(var_name);
}

Uint VariableManager::offset(const std::string& var_name) const
{
  return m_implementation->offset(var_name);
}

Uint VariableManager::nb_dof() const
{
  return m_implementation->m_nbdofs;
}

VariableManager::VariableTypesT VariableManager::variable_type(const std::string& var_name) const
{
  Implementation::VarTypesT::const_iterator result = m_implementation->m_variable_types.find(var_name);
  if(result == m_implementation->m_variable_types.end())
    throw ValueNotFound(FromHere(), "The type for variable " + var_name + " was not found in " + uri().string());
  
  return result->second;
}

void VariableManager::state_fields(std::vector< std::string >& fieldlist) const
{
  BOOST_FOREACH(std::string var_name, m_implementation->m_state_variables)
  {
    const std::string field_name = m_options[m_implementation->field_property_name(var_name)].value_str();
    fieldlist.push_back(field_name);
  }
}

void VariableManager::field_specification(std::map< std::string, std::string >& fields)
{
  m_implementation->field_specification(fields);
}


////////////////////////////////////////////////////////////////////////////////

} // Physics
} // CF
