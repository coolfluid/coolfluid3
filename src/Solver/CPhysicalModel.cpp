// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////

#include "Common/BasicExceptions.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionT.hpp"
#include "Common/URI.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CNodes.hpp"

#include "CPhysicalModel.hpp"



namespace CF {
namespace Solver {

using namespace Common;
using namespace Mesh;

ComponentBuilder < CPhysicalModel, Component, LibSolver > CPhysicalModel_Builder;

////////////////////////////////////////////////////////////////////////////////

CPhysicalModel::CPhysicalModel(const std::string& name) : Component(name),
  m_type("null"),
  m_dim(0u),
  m_nbdofs(0u)
{
  // options
  // Mesh to use
  m_mesh_option = boost::dynamic_pointer_cast< OptionComponent<CMesh> >(m_options.add_option< OptionComponent<CMesh> >("mesh", "Mesh", "The mesh that holds the geometry and fields", URI()));
  m_mesh_option.lock()->attach_trigger( boost::bind(&CPhysicalModel::trigger_mesh, this) );
  mark_basic();

  /// @todo later this will be removed when the physical model stops beign so generic

  m_options.add_option<OptionT <std::string> >("Type",
                                           "Type of the physical model (serves to identify the model)",
                                           "null")
      ->mark_basic()
      ->link_to(&m_type);

  m_options.add_option<OptionT <Uint> >("Dimensions",
                                           "Dimensionality of the problem, i.e. the number of components for the spatial coordinates",
                                           0u)
      ->mark_basic()
      ->link_to(&m_dim);

  m_options.add_option<OptionT <Uint> >("DOFs",
                                           "Degrees of freedom",
                                           0u)
      ->mark_basic()
      ->link_to(&m_nbdofs);

  m_options.add_option(OptionT<std::string>::create("solution_state","Solution State","Component describing the solution state",std::string("")))
      ->mark_basic()
      ->attach_trigger( boost::bind(&CPhysicalModel::build_solution_state, this) );
}

bool CPhysicalModel::is_state_variable(const std::string& var_name) const
{
  return m_variable_offsets.count(var_name);
}

Uint CPhysicalModel::offset(const std::string& var_name) const
{
  std::map<std::string, Uint>::const_iterator it = m_variable_offsets.find(var_name);
  if(it == m_variable_offsets.end())
    throw Common::ValueNotFound(FromHere(), "Offset for variable " + var_name + " not found. Not an equation variable?");

  return it->second;
}


void CPhysicalModel::register_variable(const std::string& name, const std::string& symbol, const VariableTypesT var_type, const bool is_state)
{
  if(m_variable_types.insert(std::make_pair(name, var_type)).second) // Check if the variable was registered
  {
    if(is_state) // Add to the state variables, if the variable belongs to the state
      m_state_variables.push_back(name);

    // Add options for changing the variable name and field name
    m_options.add_option< OptionT<std::string> >(name + std::string("FieldName"), "Field name for variable " + name, name);
    m_options.add_option< OptionT<std::string> >(name + std::string("VariableName"), "Variable name for variable " + name, symbol);
  }
}

Option& CPhysicalModel::field_option(const std::string& name)
{
  return option(name + std::string("FieldName"));
}

Option& CPhysicalModel::variable_option(const std::string& name)
{
  return option(name + std::string("VariableName"));
}


void CPhysicalModel::create_fields()
{
  CMesh& mesh = m_mesh_option.lock()->component();

  for(VarTypesT::const_iterator it = m_variable_types.begin(); it != m_variable_types.end(); ++it)
  {
    const std::string internal_name = it->first;
    m_field_names[internal_name]    = option(internal_name + std::string("FieldName")   ).value_str();
    m_variable_names[internal_name] = option(internal_name + std::string("VariableName")).value_str();
  }

  // Initialize
  m_solution_fields.clear(); m_solution_fields.reserve(m_variable_offsets.size());
  m_solution_variables.clear(); m_solution_variables.reserve(m_variable_offsets.size());
  m_solution_sizes.clear(); m_solution_sizes.reserve(m_variable_offsets.size());
  m_nbdofs = 0;

  // Calculate offset and nb_dofs
  boost_foreach(const std::string& eqn_var_name, m_state_variables)
  {
    const Uint var_size = m_variable_types[eqn_var_name] == SCALAR ? 1 : m_dim;
    m_variable_offsets[eqn_var_name] = m_nbdofs;
    m_nbdofs += var_size;

    // Update data arrays for incrementing the solution
    m_solution_fields.push_back(m_field_names[eqn_var_name]);
    m_solution_variables.push_back(m_variable_names[eqn_var_name]);
    m_solution_sizes.push_back(var_size);
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

void CPhysicalModel::set_mesh(CMesh& mesh)
{
  m_mesh_option.lock()->change_value(mesh.uri());
}


void CPhysicalModel::trigger_mesh()
{
  m_dim = m_mesh_option.lock()->component().dimension();
  create_fields();
}

////////////////////////////////////////////////////////////////////////////////

CPhysicalModel::~CPhysicalModel()
{
}

////////////////////////////////////////////////////////////////////////////////

void CPhysicalModel::build_solution_state()
{
  if (is_not_null(m_solution_state))
    remove_component(*m_solution_state);
  m_solution_state = create_component( "solution_state" , option("solution_state").value_str() ).as_ptr<State>();
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
