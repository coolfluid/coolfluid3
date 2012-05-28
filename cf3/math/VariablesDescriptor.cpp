// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////

#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/regex.hpp>
#include <boost/bind.hpp>

#include "common/Builder.hpp"
#include "common/Foreach.hpp"
#include "common/Log.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionList.hpp"
#include "common/Tags.hpp"

#include "math/VariablesDescriptor.hpp"

namespace cf3 {
namespace math {

using namespace common;

////////////////////////////////////////////////////////////////////////////////

ComponentBuilder< VariablesDescriptor, Component, LibMath > VariablesDescriptor_Builder;

////////////////////////////////////////////////////////////////////////////////

struct VariablesDescriptor::Implementation
{
  Implementation(Component& component) :
    m_component(component),
    m_dim(0u)
  {
    m_component.options().add(common::Tags::dimension(), 0u)
      .pretty_name("Dimension")
      .description("Dimension of the problem, i.e. the number of components for the spatial coordinates")
      .mark_basic()
      .link_to(&m_dim)
      .attach_trigger(boost::bind(&Implementation::trigger_dimensions, this));
  }

  //////////////// Interface implementation /////////////////////

  void push_back(const std::string& name, const VariablesDescriptor::Dimensionalities::Type type)
  {
    // Only proceed if the variable did not exist already
    if(!m_indices.insert(std::make_pair(name, m_types.size())).second)
    {
      CFdebug << "Ignoring double registration of variable " << name << CFendl;
      return;
    }

    CFdebug << "Registering variable " << name << " at position " << m_types.size() << CFendl;

    m_types.push_back(type);
    m_offsets.push_back(m_size);
    m_internal_names.push_back(name);
    m_user_names.push_back(name);

    m_component.options().add(variable_property_name(name), name)
        .pretty_name(name + std::string(" Variable Name"))
        .description("Variable name for variable " + name)
        .link_to(&m_user_names.back());

    m_size += to_size(type);
  }

  void push_back(const std::string& name, const Uint nb_vars)
  {
    if (nb_vars == 1)
    {
      push_back(name,VariablesDescriptor::Dimensionalities::SCALAR);
    }
    else
    {
      for (Uint i=0; i<nb_vars; ++i)
      {
        push_back(name+"["+to_str(i)+"]",VariablesDescriptor::Dimensionalities::SCALAR);
      }
    }
  }

  Uint nb_vars() const
  {
    return m_indices.size();
  }

  Uint size() const
  {
    if(!m_dim)
      throw SetupError(FromHere(), "Attempt to get total size for " + m_component.uri().string() + " before dimension is configured");

    return m_size;
  }

  Uint size(const std::string& name) const
  {
    if(!m_dim)
      throw SetupError(FromHere(), "Attempt to get dimension for variable " + name + " in " + m_component.uri().string() + " before dimension is configured");

    return to_size(m_types[checked_index(name)]);
  }

  Uint offset(const std::string& name) const
  {
    if(!m_dim)
      throw SetupError(FromHere(), "Attempt to get offset for variable " + name + " in " + m_component.uri().string() + " before dimension is configured");

    return m_offsets[checked_index(name)];
  }

  Uint offset(const Uint var_nb) const
  {
    if(!m_dim)
      throw SetupError(FromHere(), "Attempt to get offset for variable " + to_str(var_nb) + " in " + m_component.uri().string() + " before dimension is configured");

    return m_offsets[var_nb];
  }

  const std::string& user_variable_name(const std::string& name) const
  {
    return m_user_names[checked_index(name)];
  }

  const std::string& user_variable_name(const Uint var_nb) const
  {
    return m_user_names[var_nb];
  }

  const std::string& internal_variable_name(const Uint var_nb) const
  {
    return m_user_names[var_nb];
  }

  bool has_variable(const std::string& vname) const
  {
    return std::find(m_user_names.begin(), m_user_names.end(), vname) != m_user_names.end();
  }

  Uint var_length(const Uint var_number) const
  {
    return to_size( m_types[var_number] );
  }

  Uint var_length(const std::string& name) const
  {
    return to_size( m_types[checked_index(name)] );
  }

  /// Implementation based on Willem Deconincks code for Field
  void set_variables(const std::string& description)
  {
    const boost::regex e_variable("([[:word:]]+)[[:space:]]*(\\[[[:space:]]*[[:word:]]+[[:space:]]*\\])?");
    const boost::regex e_scalar  ("((s(cal(ar)?)?)?)|1"     ,boost::regex::perl|boost::regex::icase);
    const boost::regex e_vector("v(ec(tor)?)?",boost::regex::perl|boost::regex::icase);
    const boost::regex e_tensor("t(ens(or)?)?",boost::regex::perl|boost::regex::icase);
    const boost::regex e_array("[2-9][0-9]*");

    std::vector<std::string> tokenized_variables;
    boost::split(tokenized_variables, description, boost::is_any_of(","));
    const Uint nb_vars = tokenized_variables.size();

    std::vector<std::string> names_to_add; names_to_add.reserve(nb_vars);
    std::vector<Dimensionalities::Type> types_to_add; types_to_add.reserve(nb_vars);

    BOOST_FOREACH(std::string var, tokenized_variables)
    {
      boost::match_results<std::string::const_iterator> what;
      if (boost::regex_search(var,what,e_variable))
      {
        names_to_add.push_back(what[1]); boost::trim(names_to_add.back());
        std::string var_type = what[2];
        boost::trim_if(var_type, boost::is_any_of(" []"));

        if(boost::regex_match(var_type,e_scalar))
        {
          types_to_add.push_back(Dimensionalities::SCALAR);
        }
        else if(boost::regex_match(var_type,e_vector))
        {
          types_to_add.push_back(Dimensionalities::VECTOR);
        }
        else if(boost::regex_match(var_type,e_tensor))
        {
          types_to_add.push_back(Dimensionalities::TENSOR);
        }
        else if(boost::regex_match(var_type,e_array)) // Match varname[n] and create n scalars as a result
        {
          Uint nb_scalars = from_str<Uint>(var_type);
          const std::string basename = names_to_add.back();
          names_to_add.back() = basename + "1";
          types_to_add.push_back(Dimensionalities::SCALAR);
          for(Uint i = 1; i != nb_scalars;)
          {
            names_to_add.push_back(basename + to_str(++i));
            types_to_add.push_back(Dimensionalities::SCALAR);
          }
        }
        else
        {
          throw ParsingFailed(FromHere(), "Type " + var_type + " deduced from " + var + " is not recognized");
        }
      }
      else
      {
        throw ParsingFailed(FromHere(), "Invalid variable: " + var);
      }
    }

    for(Uint i = 0; i != names_to_add.size(); ++i)
    {
      push_back(names_to_add[i], types_to_add[i]);
    }
  }

  std::string description() const
  {
//    if(!m_dim)
//      throw SetupError(FromHere(), "Attempt to get field description in " + m_component.uri().string() + " before dimension is configured");

    const Uint nb_vars = m_indices.size();

    // Create a string with the description of the variables
    std::stringstream result_str;
    for(Uint i = 0; i != nb_vars; ++i)
    {
      result_str << (i == 0 ? "" : ",") << m_user_names[i] << "[" << m_types[i] << "]";
    }

    return result_str.str();
  }

  void prefix_variable_names(const std::string& prefix)
  {
    boost_foreach(std::string& name, m_user_names)
    {
      name = prefix+name;
    }

    std::vector<Uint> indices;                 indices.reserve(m_indices.size());
    std::vector<std::string> internal_names;   internal_names.reserve(m_indices.size());
    foreach_container((const std::string& internal_name) (const Uint idx) , m_indices)
    {
      internal_names.push_back(internal_name);
      indices.push_back(idx);
    }

    for (Uint i=0; i<internal_names.size(); ++i)
    {
      m_indices.erase(internal_names[i]);
      m_indices[prefix+internal_names[i]] = indices[i];
    }
  }

  ///////////// Helper functions ////////

  /// Convert the dimensionality type to a real size
  Uint to_size(Dimensionalities::Type dim_type) const
  {
    // special cases
    if(dim_type == Dimensionalities::VECTOR)
      return m_dim;

    if(dim_type == Dimensionalities::TENSOR)
      return m_dim*m_dim;

    // all others can be converted directly
    int converted = static_cast<int>(dim_type);
    cf3_assert(converted > 0);

    return static_cast<Uint>(converted);
  }

  void trigger_dimensions()
  {
    // Recalculate size and offsets
    m_size = 0;
    const Uint nb_vars = m_indices.size();
    for(Uint i = 0; i != nb_vars; ++i)
    {
      m_offsets[i] = m_size;
      m_size += to_size(m_types[i]);
    }
  }

  std::string variable_property_name(std::string var_name)
  {
    /// @note from Willem Deconinck: following line is commented because this forbids case sensitive variable names
    // boost::to_lower(var_name);
    return var_name + "_variable_name";
  }

  /// Index for the given internal name, throwing a nice error if it's not found
  Uint checked_index(const std::string& name) const
  {
    IndexMapT::const_iterator found = m_indices.find(name);
    if(found != m_indices.end())
      return found->second;

    std::stringstream message;
    message << "Variable with internal name " << name << " was not found in descriptor " << m_component.uri().string() << std::endl;
    message << "Possible internal names:" << std::endl;
    foreach_container( (const std::string& name) (const Uint idx) , m_indices)
    {
      message << "   - " << idx << "\t: " << name << std::endl;
    }

    throw ValueNotFound(FromHere(), message.str());
  }

  /////////////// data //////////////

  Component& m_component;

  /// dimension of physics
  Uint m_dim;

  /// Total size
  Uint m_size;

  /// Mapping from variable internal name to index in the vectors
  typedef std::map<std::string, Uint> IndexMapT;
  IndexMapT m_indices;

  /// Type of each variable
  typedef std::vector<Dimensionalities::Type> VarTypesT;
  VarTypesT m_types;

  /// Offsets for the variables
  std::vector<Uint> m_offsets;

  /// User defined variable names
  std::vector<std::string> m_user_names;
  std::vector<std::string> m_internal_names;

};

////////////////////////////////////////////////////////////////////////////////

VariablesDescriptor::Dimensionalities::Convert::Convert()
{
  all_fwd = boost::assign::map_list_of
      ( VariablesDescriptor::Dimensionalities::SCALAR,    "scalar" )
      ( VariablesDescriptor::Dimensionalities::VECTOR,    "vector" )
      ( VariablesDescriptor::Dimensionalities::TENSOR,    "tensor" );

  all_rev = boost::assign::map_list_of
      ("scalar", VariablesDescriptor::Dimensionalities::SCALAR)
      ("vector", VariablesDescriptor::Dimensionalities::VECTOR)
      ("tensor", VariablesDescriptor::Dimensionalities::TENSOR);
}

std::ostream& operator<< ( std::ostream& os, const VariablesDescriptor::Dimensionalities::Type& in )
{
  os << VariablesDescriptor::Dimensionalities::Convert::instance().to_str(in);
  return os;
}

std::istream& operator>> (std::istream& is, VariablesDescriptor::Dimensionalities::Type& in )
{
  std::string tmp;
  is >> tmp;
  in = VariablesDescriptor::Dimensionalities::Convert::instance().to_enum(tmp);
  return is;
}

////////////////////////////////////////////////////////////////////////////////

VariablesDescriptor::VariablesDescriptor( const std::string& name ) :
  Component(name),
  m_implementation(new Implementation(*this))
{
}

VariablesDescriptor::~VariablesDescriptor()
{
}

////////////////////////////////////////////////////////////////////////////////

void VariablesDescriptor::push_back(const std::string& name, const VariablesDescriptor::Dimensionalities::Type type)
{
  m_implementation->push_back(name, type);
}

////////////////////////////////////////////////////////////////////////////////

void VariablesDescriptor::push_back(const std::string& name, const Uint nb_vars)
{
  m_implementation->push_back(name, nb_vars);
}

////////////////////////////////////////////////////////////////////////////////

Uint VariablesDescriptor::nb_vars() const
{
  return m_implementation->nb_vars();
}

////////////////////////////////////////////////////////////////////////////////

Uint VariablesDescriptor::size() const
{
  return m_implementation->size();
}

////////////////////////////////////////////////////////////////////////////////

Uint VariablesDescriptor::size(const std::string& name) const
{
  return m_implementation->size(name);
}

////////////////////////////////////////////////////////////////////////////////

Uint VariablesDescriptor::offset(const std::string& name) const
{
  return m_implementation->offset(name);
}

////////////////////////////////////////////////////////////////////////////////

Uint VariablesDescriptor::offset(const Uint var_number) const
{
  return m_implementation->offset(var_number);
}

////////////////////////////////////////////////////////////////////////////////

bool VariablesDescriptor::has_variable(const std::string& name) const
{
  return m_implementation->has_variable(name);
}

////////////////////////////////////////////////////////////////////////////////

VariablesDescriptor::Dimensionalities::Type VariablesDescriptor::dimensionality(const std::string& name) const
{
  return m_implementation->m_types[m_implementation->checked_index(name)];
}


////////////////////////////////////////////////////////////////////////////////

Uint VariablesDescriptor::var_number ( const std::string& name ) const
{
  return m_implementation->checked_index(name);
}

////////////////////////////////////////////////////////////////////////////////

Uint VariablesDescriptor::var_length ( const std::string& name ) const
{
  return m_implementation->var_length(name);
}

////////////////////////////////////////////////////////////////////////////////


Uint VariablesDescriptor::var_length ( const Uint var_number ) const
{
  return m_implementation->var_length(var_number);
}

////////////////////////////////////////////////////////////////////////////////

const std::string& VariablesDescriptor::user_variable_name(const std::string& name) const
{
  return m_implementation->user_variable_name(name);
}

////////////////////////////////////////////////////////////////////////////////

const std::string& VariablesDescriptor::user_variable_name(const Uint var_nb) const
{
  return m_implementation->user_variable_name(var_nb);
}

////////////////////////////////////////////////////////////////////////////////

const std::string& VariablesDescriptor::internal_variable_name(const Uint var_nb) const
{
  return m_implementation->internal_variable_name(var_nb);
}

////////////////////////////////////////////////////////////////////////////////

std::string VariablesDescriptor::description() const
{
  return m_implementation->description();
}

////////////////////////////////////////////////////////////////////////////////

void VariablesDescriptor::set_variables(const std::string& description)
{
  m_implementation->set_variables(description);
}


void VariablesDescriptor::set_variables(const std::string& description, const Uint dimension)
{
  options().set(common::Tags::dimension(), dimension);
  m_implementation->set_variables(description);
}

////////////////////////////////////////////////////////////////////////////////

void VariablesDescriptor::prefix_variable_names(const std::string& prefix)
{
  m_implementation->prefix_variable_names(prefix);
}

////////////////////////////////////////////////////////////////////////////////

} // math
} // cf3
