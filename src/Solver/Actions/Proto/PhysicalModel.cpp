// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "Common/CF.hpp"
#include "Common/Foreach.hpp"
#include "Common/PropertyList.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CNodes.hpp"

#include "Solver/CEigenLSS.hpp"

#include "PhysicalModel.hpp"

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

using namespace CF::Common;
using namespace CF::Mesh;

PhysicalModel::PhysicalModel() : m_nb_dofs(0)
{
}


Uint PhysicalModel::nb_dofs() const
{
  return m_nb_dofs;
}

bool PhysicalModel::is_equation_variable(const std::string& var_name) const
{
  return m_variable_offsets.find(var_name) != m_variable_offsets.end();
}

Uint PhysicalModel::offset(const std::string& var_name) const
{ 
  const std::map<std::string, Uint>::const_iterator it = m_variable_offsets.find(var_name);
  if(it == m_variable_offsets.end())
    throw ValueNotFound(FromHere(), "Variable " + var_name + " does not represent a state");
  return it->second;
}

void PhysicalModel::register_variable(const std::string& var_name, const VariableTypesT var_type, const bool is_equation_var)
{
  if(m_variable_types.insert(std::make_pair(var_name, var_type)).second && is_equation_var)
  {
    m_equation_variables.push_back(var_name);
  }
}

void PhysicalModel::register_variable(const CF::Solver::Actions::Proto::ScalarField& var, const bool is_equation_var)
{
  if(m_variable_types.insert(std::make_pair(var.internal_name(), SCALAR)).second)
  {
    m_field_names[var.internal_name()] = var.field_name;
    m_variable_names[var.internal_name()] = var.var_name;
    if(is_equation_var)
      m_equation_variables.push_back(var.internal_name());
  }
}

void PhysicalModel::register_variable(const CF::Solver::Actions::Proto::VectorField& var, const bool is_equation_var)
{
  if(m_variable_types.insert(std::make_pair(var.internal_name(), VECTOR)).second)
  {
    m_field_names[var.internal_name()] = var.field_name;
    m_variable_names[var.internal_name()] = var.var_name;
    if(is_equation_var)
      m_equation_variables.push_back(var.internal_name());
  }
}


void PhysicalModel::create_fields(CMesh& mesh)
{
  // dimension of the problem
  const Uint dim = mesh.topology().nodes().coordinates().row_size();
  
  // Initialize
  m_solution_fields.clear(); m_solution_fields.reserve(m_variable_offsets.size());
  m_solution_variables.clear(); m_solution_variables.reserve(m_variable_offsets.size());
  m_solution_sizes.clear(); m_solution_sizes.reserve(m_variable_offsets.size());
  m_nb_dofs = 0;
  
  // Calculate offset and nb_dofs
  boost_foreach(const std::string& eqn_var_name, m_equation_variables)
  {
    const Uint var_size = m_variable_types[eqn_var_name] == SCALAR ? 1 : dim;
    m_variable_offsets[eqn_var_name] = m_nb_dofs;
    m_nb_dofs += var_size;
    
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
      fd_var_types.push_back(static_cast<Mesh::CField::VarType>(var_it->second == SCALAR ? 1 : dim));
      
      if(existing_field) // If the field exists, check if it is compatible
      {
        if(!existing_field->has_variable(fd_var_names.back()) // Variable doesn't exist
           || (existing_field->has_variable(fd_var_names.back()) && existing_field->var_type(fd_var_names.back()) != fd_var_types.back())) // Variable exists but size is incorrect
          throw Common::ValueExists(FromHere(), "Field with name " + fd_name + " exists, but is incompatible with the requested solution.");
      }
    }
    
    if(!existing_field)
      mesh.create_field2(fd_name, CField::Basis::POINT_BASED, fd_var_names, fd_var_types);
  }
}

void PhysicalModel::create_fields(CMesh& mesh, const CF::Common::PropertyList& properties)
{
  get_names(properties);
  create_fields(mesh);
}

void PhysicalModel::get_names(const CF::Common::PropertyList& properties)
{
  for(VarTypesT::const_iterator it = m_variable_types.begin(); it != m_variable_types.end(); ++it)
  {
    const std::string internal_name = it->first;
    m_field_names[internal_name]    = properties[internal_name + std::string("FieldName")   ].value_str();
    m_variable_names[internal_name] = properties[internal_name + std::string("VariableName")].value_str();
  }
}


void PhysicalModel::update_fields(CMesh& solution_mesh, const RealVector& solution)
{
  increment_solution(solution, m_solution_fields, m_solution_variables, m_solution_sizes, solution_mesh);
}

void PhysicalModel::clear()
{
  m_variable_offsets.clear();
  m_variable_types.clear();
  m_equation_variables.clear();
  m_nb_dofs = 0;
}

 
} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF
