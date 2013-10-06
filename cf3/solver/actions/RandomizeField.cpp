// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real_distribution.hpp>

#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/List.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/Space.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/Connectivity.hpp"

#include "solver/actions/RandomizeField.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < RandomizeField, common::Action, LibActions > RandomizeField_Builder;

///////////////////////////////////////////////////////////////////////////////////////

namespace detail
{
  // Helper to get a vector option
  std::vector<Real> get_vector(const common::Component& comp, const std::string& name, const Uint expected_size)
  {
    std::vector<Real> result = comp.options().value< std::vector<Real> >(name);
    if(result.size() != expected_size)
      throw common::SetupError(FromHere(), "Option " + name + " was expected to be of size " + common::to_str(expected_size) + " but was found to have size " + common::to_str(result.size()));
    
    return result;
  }
  
}

RandomizeField::RandomizeField ( const std::string& name ) :
  common::Action(name)
{
  options().add("field", Handle<mesh::Field>())
    .pretty_name("Field")
    .description("Field to modify")
    .mark_basic();
    
  options().add("variable_name", "VariableNameNotSet")
    .pretty_name("Variable Name")
    .description("Name of the variable in the field that is to be randomized")
    .mark_basic();
    
  options().add("maximum_variations", std::vector<Real>())
    .pretty_name("Maximum Variations")
    .description("Maximum variation (in both directions) for each component")
    .mark_basic();
  
  options().add("maximum_values", std::vector<Real>())
    .pretty_name("Maximum Values")
    .description("Maximum value for each component")
    .mark_basic();
    
  options().add("minimum_values", std::vector<Real>())
    .pretty_name("Minimum Values")
    .description("Minimum value for each component")
    .mark_basic();
    
  options().add("reference_component", 0u)
    .pretty_name("Reference Component")
    .description("Component to use as the base for the scaling")
    .mark_basic();
    
  options().add("seed", 0u)
    .pretty_name("Seed")
    .description("Seed for the random generator.");    
}

/////////////////////////////////////////////////////////////////////////////////////

void RandomizeField::execute()
{
  Handle<mesh::Field> field_handle = options().value< Handle<mesh::Field> >("field");
  if(is_null( field_handle ))
    throw common::SetupError(FromHere(), "field option is not set for " + uri().path());
  
  mesh::Field& field = *field_handle;
  
  const std::string var_name = options().value<std::string>("variable_name");
  const Uint reference_component = options().value<Uint>("reference_component");
  const Uint var_begin = field_handle->var_offset(var_name);
  const Uint var_length = field_handle->var_length(var_name);
  
  if(reference_component > var_length)
    throw common::SetupError(FromHere(), "Value for reference_component is out of range");
  
  const std::vector<Real> maximum_variations = detail::get_vector(*this, "maximum_variations", var_length);
  const std::vector<Real> maximum_values = detail::get_vector(*this, "maximum_values", var_length);
  const std::vector<Real> minimum_values = detail::get_vector(*this, "minimum_values", var_length);
  
  boost::random::mt19937 gen(common::PE::Comm::instance().rank() + options().value<Uint>("seed"));
  boost::random::uniform_real_distribution<Real> dist(-1., 1.);
  
  const Uint nb_rows = field.size();
  for(Uint i = 0; i != nb_rows; ++i)
  {
    const Real ref_val = field[i][var_begin + reference_component];
    for(Uint j = 0; j != var_length; ++j)
    {
      Real& value = field[i][j+var_begin];
      const Real new_value = value + ref_val*dist(gen)*maximum_variations[j];
      value = std::max(std::min(new_value, maximum_values[j]), minimum_values[j]);
    }
  }
  
  // Sync periodic nodes
  Handle< common::List<Uint> > periodic_links_nodes_h(field.parent()->get_child("periodic_links_nodes"));
  Handle< common::List<bool> > periodic_links_active_h(field.parent()->get_child("periodic_links_active"));
  if(is_not_null(periodic_links_active_h) && is_not_null(periodic_links_nodes_h))
  {
    const common::List<Uint>& periodic_links_nodes = *periodic_links_nodes_h;
    const common::List<bool>& periodic_links_active = *periodic_links_active_h;
    cf3_assert(nb_rows == periodic_links_nodes.size());
    for(Uint i = 0; i != nb_rows; ++i)
    {
      Uint target_node = i;
      while(periodic_links_active[target_node])
      {
        target_node = periodic_links_nodes[target_node];
      }
      if(i != target_node)
      {
        for(Uint j = 0; j != var_length; ++j)
          field[i][var_begin+j] = field[target_node][var_begin+j];
      }
    }
  }
  
  // Parallel sync
  if(common::PE::Comm::instance().is_active())
  {
    field.parallelize();
    field.synchronize();
  }
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////////

