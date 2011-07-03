// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////

#include "Common/BasicExceptions.hpp"
#include "Common/CBuilder.hpp"
#include "Common/Foreach.hpp"
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

struct CPhysicalModel::Implementation
{
  Implementation(Component& component) :
    m_component(component),
    m_type("null"),
    m_dim(0u),
    m_nbdofs(0u)
  {
    
    m_component.options().add_option( OptionComponent<CMesh>::create("mesh", "Mesh"
                                                                   "Mesh this physical model applies to",
                                                                   &m_mesh))
      ->mark_basic()
      ->attach_trigger(boost::bind(&Implementation::trigger_mesh, this));
    
    /// @todo later this will be removed when the physical model stops beign so generic

    m_component.options().add_option<OptionT <std::string> >("Type",
                                            "Type of the physical model (serves to identify the model)",
                                            "null")
        ->mark_basic()
        ->link_to(&m_type);

    m_component.options().add_option<OptionT <Uint> >("Dimensions",
                                            "Dimensionality of the problem, i.e. the number of components for the spatial coordinates",
                                            0u)
        ->mark_basic()
        ->link_to(&m_dim);

    m_component.options().add_option<OptionT <Uint> >("DOFs",
                                            "Degrees of freedom",
                                            0u)
        ->mark_basic()
        ->link_to(&m_nbdofs);

    m_component.options().add_option(OptionT<std::string>::create("solution_state","Solution State","Component describing the solution state",std::string("")))
        ->mark_basic()
        ->attach_trigger( boost::bind(&Implementation::build_solution_state, this) );
  }
  
  void build_solution_state()
  {
    if (is_not_null(m_solution_state))
      m_component.remove_component(*m_solution_state);
    m_solution_state = m_component.create_component( "solution_state" , m_component.option("solution_state").value_str() ).as_ptr<State>();
  }
  
  void trigger_mesh()
  {
    mesh_checked().option("dimension").attach_trigger(boost::bind(&Implementation::trigger_dimension, this));
    trigger_dimension();
  }
  
  void trigger_dimension()
  {
    m_dim = mesh_checked().dimension();
    m_nb_nodes = m_dim ? mesh_checked().topology().nodes().size() : 0;
    if(m_dim && m_nb_nodes)
      create_fields();
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


  void register_variable(const std::string& name, std::string& symbol, std::string& field_name, const CF::Solver::CPhysicalModel::VariableTypesT var_type, const bool is_state)
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
      m_component.options().add_option< OptionT<std::string> >(name + std::string("FieldName"), "Field name for variable " + name, field_name)->link_to(&field_name);
      m_component.options().add_option< OptionT<std::string> >(name + std::string("VariableName"), "Variable name for variable " + name, symbol)->link_to(&symbol);
    }
    else
    {
      if(is_state && !std::count(m_state_variables.begin(), m_state_variables.end(), name))
      {
        m_state_variables.push_back(name);
        m_variable_offsets[name] = m_nbdofs;
        m_nbdofs += var_type == SCALAR ? 1 : m_dim;
      }
      
      m_component.option(name + "FieldName").link_to(&field_name);
      m_component.option(name + "VariableName").link_to(&symbol);
    }
  }

  void create_fields()
  {
    CMesh& mesh = mesh_checked();
    cf_assert(m_dim);

    for(VarTypesT::const_iterator it = m_variable_types.begin(); it != m_variable_types.end(); ++it)
    {
      const std::string internal_name = it->first;
      m_field_names[internal_name]    = m_component.option(internal_name + std::string("FieldName")   ).value_str();
      m_variable_names[internal_name] = m_component.option(internal_name + std::string("VariableName")).value_str();
    }

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
  
  CMesh& mesh_checked()
  {
    if(m_mesh.expired())
      throw SetupError(FromHere(), "Mesh is not set for physical model at " + m_component.uri().string());
    
    return *m_mesh.lock();
  }

  Component& m_component;
  
  /// type of the physcial model
  std::string m_type;

  /// dimensionality of physics
  Uint m_dim;

  /// number of degrees of freedom
  Uint m_nbdofs;
  
  /// Number of nodes in the mesh
  Uint m_nb_nodes;

  State::Ptr m_solution_state;
  
  /// Option for referring to the mesh
  boost::weak_ptr< Common::OptionComponent<Mesh::CMesh> > m_mesh_option;
  
  /// Offset of each equation variable, i.e. in V (vector of u and v) and p, V has offset 0, and p has offset 2 when the order is uvp in the global system
  std::map<std::string, Uint> m_variable_offsets;
  
  /// Ordered list of the equation variables
  std::vector<std::string> m_state_variables;
  
  /// Type of each variable
  typedef std::map<std::string, VariableTypesT> VarTypesT;
  VarTypesT m_variable_types;
  
  /// Storage for field and variable names
  std::map<std::string, std::string> m_field_names;
  std::map<std::string, std::string> m_variable_names;
  
  boost::weak_ptr<CMesh> m_mesh;
};

////////////////////////////////////////////////////////////////////////////////

CPhysicalModel::CPhysicalModel(const std::string& name) :
  Component(name),
  m_implementation(new Implementation(*this))
{
}

CPhysicalModel::~CPhysicalModel()
{
}

void CPhysicalModel::create_fields()
{
  m_implementation->create_fields();
}

Uint CPhysicalModel::dimensions() const
{
  return m_implementation->m_dim;
}

bool CPhysicalModel::is_state_variable(const std::string& var_name) const
{
  return m_implementation->is_state_variable(var_name);
}

Uint CPhysicalModel::nb_dof() const
{
  return m_implementation->m_nbdofs;
}

Uint CPhysicalModel::nb_nodes() const
{
  return m_implementation->m_nb_nodes;
}

Uint CPhysicalModel::offset(const std::string& var_name) const
{
  return m_implementation->offset(var_name);
}

void CPhysicalModel::register_variable(const std::string& name, std::string& symbol, std::string& field_name, const CPhysicalModel::VariableTypesT var_type, const bool is_state)
{
  m_implementation->register_variable(name, symbol, field_name, var_type, is_state);
}

const State& CPhysicalModel::solution_state() const
{
  return *m_implementation->m_solution_state;
}

CPhysicalModel::VariableTypesT CPhysicalModel::variable_type(const std::string& name) const
{
  Implementation::VarTypesT::const_iterator result = m_implementation->m_variable_types.find(name);
  if(result == m_implementation->m_variable_types.end())
    throw ValueNotFound(FromHere(), "The type for variable " + name + " was not found in " + uri().string());
  
  return result->second;
}


std::string CPhysicalModel::type() const
{
  return m_implementation->m_type;
}

void CPhysicalModel::state_fields(std::vector< URI >& fieldlist) const
{
  CMesh& mesh = m_implementation->mesh_checked();
  BOOST_FOREACH(const std::string& var_name, m_implementation->m_state_variables)
  {
    const std::string field_name = m_options[var_name + "FieldName"].value_str();
    fieldlist.push_back(mesh.get_child(field_name).uri());
  }
}


////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
