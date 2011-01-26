// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"

#include "Mesh/Actions/CInfo.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {
  
  using namespace Common;
  
  class IsElementRegion
  {
  public:
    IsElementRegion () {}
    
    bool operator()(const Component& component)
    {
      return !find_components<CTable<Uint> >(component).empty() && !find_components<CElements>(component).empty();
    }
    
  }; // IsElementRegion
  
  class IsGroup
  {
  public:
    IsGroup () {}
    
    bool operator()(const Component& component)
    {
      return count(find_components_with_filter<CRegion>(component,m_isElement));
    }
    
  private:
    IsElementRegion m_isElement;
  }; // IsGroup
  
////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CInfo, CMeshTransformer, LibActions> CInfo_Builder;

//////////////////////////////////////////////////////////////////////////////

CInfo::CInfo( const std::string& name )
: CMeshTransformer(name)
{
   
	properties()["brief"] = std::string("Print information of the mesh");
	std::string desc;
	desc = 
  "  Usage: Info \n\n"
  "          Information given: internal mesh hierarchy,\n"
  "      element distribution for each region, and element type"; 
	properties()["description"] = desc;
}

/////////////////////////////////////////////////////////////////////////////

std::string CInfo::brief_description() const
{
  return properties()["brief"].value<std::string>();
}

/////////////////////////////////////////////////////////////////////////////

  
std::string CInfo::help() const
{
  return "  " + properties()["brief"].value<std::string>() + "\n" + properties()["description"].value<std::string>();
}  
  
/////////////////////////////////////////////////////////////////////////////

void CInfo::transform(const CMesh::Ptr& mesh, const std::vector<std::string>& args)
{

  m_mesh = mesh;

  CFinfo << "Element distribution:" << CFendl;
  BOOST_FOREACH( const CRegion& region, find_components_with_filter<CRegion>(*m_mesh,IsComponentTrue()))
  {
    CFinfo << print_region_tree(region) << CFflush;
  }  

  CFinfo << "Fields:" << CFendl;
  BOOST_FOREACH( const CField& region, find_components_with_filter<CField>(*m_mesh,IsComponentTrue()))
  {
    CFinfo << print_field_tree(region) << CFflush;
  }  
}

//////////////////////////////////////////////////////////////////////////////

std::string CInfo::print_region_tree(const CRegion& region, Uint level)
{
  std::string tree;
    
  for (Uint i=0; i<level; i++)
    tree += "    ";
  tree += region.name() + " (" + String::to_str<Uint>(region.recursive_elements_count()) +  ")\n";
  
  tree += print_elements(region,level+1);
  
  BOOST_FOREACH( const CRegion& subregion, find_components_with_filter<CRegion>(region,IsComponentTrue()))
  {
    tree += print_region_tree(subregion,level+1);
  }
  return tree;    
}

//////////////////////////////////////////////////////////////////////////////

std::string CInfo::print_field_tree(const CField& field, Uint level)
{
  std::string tree;
  
  for (Uint i=0; i<level; i++)
    tree += "    ";
  tree += field.name() + " (" + String::to_str<Uint>(field.recursive_elements_count()) +  ")\n";
  
  tree += print_elements(field,level+1);
  
  BOOST_FOREACH( const CField& subfield, find_components_with_filter<CField>(field,IsComponentTrue()))
  {
    tree += print_field_tree(subfield,level+1);
  }
  return tree;    
}

//////////////////////////////////////////////////////////////////////////////

std::string CInfo::print_elements(const Component& region, Uint level)
{
  std::string tree;
  BOOST_FOREACH( const CElements& elements_region, find_components<CElements>(region))
  {
    for (Uint i=0; i<level; i++)
      tree += "    ";
    std::string dimensionality = IsElementsVolume()(elements_region) ? "volume" : "surface";
    tree += elements_region.name() + " -- " + dimensionality + "  (" + String::to_str<Uint>(elements_region.size()) +  ")\n";
  }
  return tree;
}

//////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // CF
