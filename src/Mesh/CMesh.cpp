// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>

#include "Common/CBuilder.hpp"
#include "Common/CLink.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/String/Conversion.hpp"

#include "Mesh/LibMesh.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CField2.hpp"

namespace CF {
namespace Mesh {

using namespace Common;
using namespace Common::String;

Common::ComponentBuilder < CMesh, Component, LibMesh > CMesh_Builder;

////////////////////////////////////////////////////////////////////////////////

CMesh::CMesh ( const std::string& name  ) :
  Component ( name )
{
  m_properties.add_property("nb_cells",Uint(0));
  m_properties.add_property("nb_nodes",Uint(0));
  m_properties.add_property("dimensionality",Uint(0));
  m_properties.add_property("dimension",Uint(0));

  mark_basic(); // by default meshes are visible
  
  m_nodes_link = create_static_component<CLink>("nodes");
  m_topology = create_static_component<CRegion>("topology");
}

////////////////////////////////////////////////////////////////////////////////

CMesh::~CMesh()
{
}

////////////////////////////////////////////////////////////////////////////////

CField2& CMesh::create_field2( const std::string& name , const std::string& base, const std::string& variables)
{
  std::vector<std::string> tokenized_variables(0);
  
  if (variables == "scalar_same_name")
  {
    tokenized_variables.push_back(name+"[scalar]");
  }
  else
  {
    typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
    boost::char_separator<char> sep(",");
    Tokenizer tokens(variables, sep);

    for (Tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
      tokenized_variables.push_back(*tok_iter);
  }


  std::vector<std::string> names;
  std::vector<std::string> types;
  BOOST_FOREACH(std::string var, tokenized_variables)
  { 
    boost::regex e_variable("([[:word:]]+)?[[:space:]]*\\[[[:space:]]*([[:word:]]+)[[:space:]]*\\]");
    
    boost::match_results<std::string::const_iterator> what;
    if (regex_search(var,what,e_variable))
    {
      names.push_back(what[1]);
      types.push_back(what[2]);
    }
    else
      throw ShouldNotBeHere(FromHere(), "No match found for VarType " + var);
  }

  CField2& field = *create_component<CField2>(name);
  field.set_topology(topology());
  field.configure_property("VarNames",names);
  field.configure_property("VarTypes",types);
  field.configure_property("FieldType",base);
  field.create_data_storage();

  return field;
}

////////////////////////////////////////////////////////////////////////////////

CField& CMesh::create_field( const std::string& name , CRegion& support, const std::vector<std::string>& variables, const CField::DataBasis basis)
{
  CField& field = *create_component<CField>(name);
  field.synchronize_with_region(support);

  std::vector<std::string> names;
  std::vector<std::string> types;
  BOOST_FOREACH(std::string var, variables)
  { 
    boost::regex e_variable("([[:word:]]+)?[[:space:]]*\\[[[:space:]]*([[:word:]]+)[[:space:]]*\\]");
    
    boost::match_results<std::string::const_iterator> what;
    if (regex_search(var,what,e_variable))
    {
      names.push_back(what[1]);
      types.push_back(what[2]);
    }
    else
      throw ShouldNotBeHere(FromHere(), "No match found for VarType " + var);
  }

  field.configure_property("VarNames",names);
  field.configure_property("VarTypes",types);
  field.create_data_storage(basis);

  return field;
}
  
CField& CMesh::create_field( const std::string& name , const std::vector<std::string>& variables, const CField::DataBasis basis)
{
  return create_field(name,topology(),variables,basis);
}

////////////////////////////////////////////////////////////////////////////////

CField& CMesh::create_field( const std::string& name , CRegion& support, const Uint size, const CField::DataBasis basis)
{
  std::vector<std::string> variables;
  if (size==1)
  {
    variables.push_back(name+"[scalar]");
  }
  else
  {
    for (Uint iVar=0; iVar<size; ++iVar)
      variables.push_back(name+to_str(iVar)+"[scalar]");
  }
  return create_field(name,support,variables,basis);
}

////////////////////////////////////////////////////////////////////////////////

CField& CMesh::create_field( const std::string& name, const Uint size, const CField::DataBasis basis )
{
  return create_field(name,topology(),size,basis);
}

////////////////////////////////////////////////////////////////////////////////

const CField& CMesh::field(const std::string& name) const
{
  return find_component_with_name<CField const>(*this,name);
}

////////////////////////////////////////////////////////////////////////////////

CField& CMesh::field(const std::string& name)
{
  return find_component_with_name<CField>(*this,name);
}

////////////////////////////////////////////////////////////////////////////////

CNodes& CMesh::nodes() 
{ 
  cf_assert( is_not_null(m_nodes_link->follow()) );
  return *m_nodes_link->follow()->as_type<CNodes>();
}

////////////////////////////////////////////////////////////////////////////////

const CNodes& CMesh::nodes() const 
{ 
  cf_assert( is_not_null(m_nodes_link->follow()) );
  return *m_nodes_link->follow()->as_type<CNodes>();
}

//////////////////////////////////////////////////////////////////////////////


} // Mesh
} // CF
