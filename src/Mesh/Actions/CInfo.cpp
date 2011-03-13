// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"
#include "Common/StringConversion.hpp"

#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/CMesh.hpp"

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
      return !find_components<CTable<Uint> >(component).empty() && !find_components<CEntities>(component).empty();
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

void CInfo::execute()
{

  CMesh& mesh = *m_mesh.lock();

  CFinfo << "Element distribution:" << CFendl;
  boost_foreach( const CRegion& region, find_components_with_filter<CRegion>(mesh,IsComponentTrue()))
  {
    CFinfo << print_region_tree(region) << CFflush;
  }  

  CFinfo << "Fields:" << CFendl;
  boost_foreach( const CField2& field, find_components<CField2>(mesh) )
  {
    CFinfo << " - " << field.name() << "  (" << CField2::Basis::Convert::instance().to_str(field.basis()) << ")" << CFendl;
    for (Uint i=0; i<field.nb_vars(); ++i)
    {
      CFinfo << "     " << field.var_name(i) << "[" << (Uint) field.var_type(i) << "]" << CFendl;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

std::string CInfo::print_region_tree(const CRegion& region, Uint level)
{
  std::string tree;
    
  for (Uint i=0; i<level; i++)
    tree += "    ";
  tree += region.name() + " (" + to_str(region.recursive_elements_count()) +  ")\n";
  
  tree += print_elements(region,level+1);
  
  boost_foreach( const CRegion& subregion, find_components_with_filter<CRegion>(region,IsComponentTrue()))
  {
    tree += print_region_tree(subregion,level+1);
  }
  return tree;    
}

//////////////////////////////////////////////////////////////////////////////

std::string CInfo::print_elements(const Component& region, Uint level)
{
  std::string tree;
  boost_foreach( const CEntities& elements_region, find_components<CEntities>(region))
  {
    for (Uint i=0; i<level; i++)
      tree += "    ";
    std::string dimensionality = elements_region.element_type().dimension() == elements_region.element_type().dimensionality() ? "volume" : "surface";
    tree += elements_region.name() + " -- " + dimensionality + "  (" + to_str(elements_region.size()) +  ")\n";
  }
  return tree;
}

//////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // CF
