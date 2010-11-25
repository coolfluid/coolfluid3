// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/regex.hpp>

#include "Common/CBuilder.hpp"
#include "Common/CLink.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/String/Conversion.hpp"

#include "Mesh/LibMesh.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/ElementType.hpp"

namespace CF {
namespace Mesh {

using namespace Common;
using namespace Common::String;

Common::ComponentBuilder < CMesh, Component, LibMesh >
CMesh_Builder ( CMesh::type_name() );

////////////////////////////////////////////////////////////////////////////////

CMesh::CMesh ( const std::string& name  ) :
  Component ( name )
{
  BuildComponent<full>().build(this);
	m_properties.add_property("nb_cells",Uint(0));
	m_properties.add_property("nb_nodes",Uint(0));
	m_properties.add_property("dimensionality",Uint(0));
	m_properties.add_property("dimension",Uint(0));
}

////////////////////////////////////////////////////////////////////////////////

CMesh::~CMesh()
{
}

////////////////////////////////////////////////////////////////////////////////

CRegion& CMesh::create_region( const std::string& name, bool ensure_unique )
{
  CRegion::Ptr new_region;
  
  
  if ( range_typed<CRegion>(*this).empty() )
  {
    new_region = create_component_type<CRegion>(name);
    new_region->add_tag("grid_zone");
  }
  else
  {
    CRegion& existing_region = get_component_typed<CRegion>(*this);
    if (existing_region.has_tag("grid_base"))
    {
//      //count howmany times the name "name(_[0-9]+)?" occurs (REGEX)
//      Uint count = 0;
//      boost::regex e(name+"(_[0-9]+)?");
//      
//      BOOST_FOREACH(const CRegion& region, range_typed<CRegion>(existing_region))
//        if (boost::regex_match(region.name(), e))
//          count++;
//
//      std::string append = (count == 0) ? "" : "_"+to_str(count);
      new_region = existing_region.create_region(name,ensure_unique).get_type<CRegion>();
			new_region->add_tag("grid_zone");
    }
    else if (existing_region.has_tag("grid_zone"))
    {
      // Create a parent region "base" for the existing region
      CRegion::Ptr base_region = create_component_type<CRegion>("base");
      base_region->add_tag("grid_base");
      existing_region.move_component(base_region);
            
//      //count howmany times the name "name(_[0-9]+)?" occurs (REGEX)
//      Uint count = 0;
//      boost::regex e(name+"(_[0-9]+)?");
//      
//      BOOST_FOREACH(const CRegion& region, range_typed<CRegion>(*base_region))
//      if (boost::regex_match(region.name(), e))
//        count++;
//
//      std::string append = (count == 0) ? "" : "_"+to_str(count);
      new_region = base_region->create_region(name,ensure_unique).get_type<CRegion>();
      new_region->add_tag("grid_zone");
    }
    else
    {
      throw ValueNotFound (FromHere(), "The existing region " + existing_region.full_path().string() + " does not have a tag \"grid_zone\" or \"grid_base\"");
    }

    
  }
  
  return *new_region;
}

////////////////////////////////////////////////////////////////////////////////

CRegion& CMesh::create_domain( const std::string& name )
{
  CRegion::Ptr new_region = get_child_type<CRegion>(name);
  if (!new_region)
  {
    new_region = create_component_type<CRegion>(name);
    new_region->add_tag("grid_base");
  }
  return *new_region;
}

////////////////////////////////////////////////////////////////////////////////


CField& CMesh::create_field( const std::string& name , CRegion& support, const std::vector<std::string>& variables, const CField::DataBasis basis)
{
	CField& field = *create_component_type<CField>(name);
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
  return create_field(name,domain(),variables,basis);
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
  return create_field(name,domain(),size,basis);
}
  
////////////////////////////////////////////////////////////////////////////////

const CRegion& CMesh::domain() const
{
  return get_component_typed<CRegion const>(*this);
}

////////////////////////////////////////////////////////////////////////////////

CRegion& CMesh::domain()
{
  return get_component_typed<CRegion>(*this);
}
  
////////////////////////////////////////////////////////////////////////////////

const CField& CMesh::field(const std::string& name) const
{
  return get_named_component_typed<CField const>(*this,name);
}

////////////////////////////////////////////////////////////////////////////////

CField& CMesh::field(const std::string& name)
{
  return get_named_component_typed<CField>(*this,name);
}

////////////////////////////////////////////////////////////////////////////////


} // Mesh
} // CF
