// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>

#include "Common/ObjectProvider.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CMeshInfo.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  
  using namespace Common;
  
  class IsElementRegion
  {
  public:
    IsElementRegion () {}
    
    bool operator()(const Component& component)
    {
      return !range_typed<CTable>(component).empty() && !range_typed<CElements>(component).empty();
    }
    
  }; // IsElementRegion
  
  class IsGroup
  {
  public:
    IsGroup () {}
    
    bool operator()(const Component& component)
    {
      return count(filtered_range_typed<CRegion>(component,m_isElement));
    }
    
  private:
    IsElementRegion m_isElement;
  }; // IsGroup
  
////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Mesh::CMeshInfo,
                         Mesh::CMeshTransformer,
                         Mesh::LibMesh,
                         NB_ARGS_1 >
CMeshInfo_Provider ( "Info" );

//////////////////////////////////////////////////////////////////////////////

CMeshInfo::CMeshInfo( const CName& name )
: CMeshTransformer(name)
{
  BUILD_COMPONENT;
}

/////////////////////////////////////////////////////////////////////////////

std::string CMeshInfo::brief_description() const
{
  return "Print information of the mesh";
}

/////////////////////////////////////////////////////////////////////////////

  
std::string CMeshInfo::help() const
{
  std::stringstream out;
 
  out << "  " << brief_description() << "\n";
  out << "  Usage: Info \n\n";
  out << "          Information given: internal mesh hierarchy,\n";
  out << "      element distribution for each region, and element type"; 
  
  return out.str();
}  
  
/////////////////////////////////////////////////////////////////////////////

void CMeshInfo::transform(const CMesh::Ptr& mesh, const std::vector<std::string>& args)
{

  m_mesh = mesh;

  CFinfo << "Element distribution:" << CFendl;
  BOOST_FOREACH( const CRegion& region, filtered_range_typed<CRegion>(*m_mesh,IsComponentTrue()))
  {
    CFinfo << print_region_tree(region) << CFflush;
  }  

  CFinfo << "Fields:" << CFendl;
  BOOST_FOREACH( const CField& region, filtered_range_typed<CField>(*m_mesh,IsComponentTrue()))
  {
    CFinfo << print_field_tree(region) << CFflush;
  }  
}

//////////////////////////////////////////////////////////////////////////////

std::string CMeshInfo::print_region_tree(const CRegion& region, Uint level)
{
  std::string tree;
    
  for (Uint i=0; i<level; i++)
    tree += "    ";
  tree += region.name() + " (" + String::to_str<Uint>(region.recursive_elements_count()) +  ")\n";
  
  tree += print_elements(region,level+1);
  
  BOOST_FOREACH( const CRegion& subregion, filtered_range_typed<CRegion>(region,IsComponentTrue()))
  {
    tree += print_region_tree(subregion,level+1);
  }
  return tree;    
}

//////////////////////////////////////////////////////////////////////////////

std::string CMeshInfo::print_field_tree(const CField& field, Uint level)
{
  std::string tree;
  
  for (Uint i=0; i<level; i++)
    tree += "    ";
  tree += field.name() + " (" + String::to_str<Uint>(field.recursive_elements_count()) +  ")\n";
  
  tree += print_elements(field,level+1);
  
  BOOST_FOREACH( const CField& subfield, filtered_range_typed<CField>(field,IsComponentTrue()))
  {
    tree += print_field_tree(subfield,level+1);
  }
  return tree;    
}

//////////////////////////////////////////////////////////////////////////////

std::string CMeshInfo::print_elements(const Component& region, Uint level)
{
  std::string tree;
  BOOST_FOREACH( const CElements& elements_region, range_typed<CElements>(region))
  {
    for (Uint i=0; i<level; i++)
      tree += "    ";
    std::string dimensionality = IsElementsVolume()(elements_region) ? "volume" : "surface";
    tree += elements_region.name() + " -- " + dimensionality + "  (" + String::to_str<Uint>(elements_region.elements_count()) +  ")\n";
  }
  return tree;
}

//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
