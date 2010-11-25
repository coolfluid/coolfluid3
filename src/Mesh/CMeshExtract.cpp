// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>

#include "Common/CBuilder.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CMeshExtract.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  
  using namespace Common;
  
  class IsGroup
  {
  public:
    IsGroup () {}
    
    bool operator()(const Component& component)
    {
      return count(range_typed<CElements>(component));
    }
  }; // IsGroup
  
  
////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Mesh::CMeshExtract,
                           Mesh::CMeshTransformer,
                           Mesh::LibMesh >
CMeshExtract_Builder;

//////////////////////////////////////////////////////////////////////////////

CMeshExtract::CMeshExtract( const std::string& name )
: CMeshTransformer(name)
{
  BUILD_COMPONENT;
}

/////////////////////////////////////////////////////////////////////////////
  
std::string CMeshExtract::brief_description() const
{
  return "Extract given regions from the mesh";
}

/////////////////////////////////////////////////////////////////////////////

std::string CMeshExtract::help() const
{
  std::stringstream out;
  out << "  " << brief_description() << "\n";
  out << "  Usage: Extract [region_name1 region_name2 ...]\n\n";
  out << "          Special cases: \"surfaces\", \"volumes\" as region_name.\n";
  out << "      A region_name can be a regular expression matched with the full path.\n"; 
  out << "  Example:\n"; 
  out << "          Given a mesh with data organized in the following way:\n"; 
  out << "      mesh\n";
  out << "            zone_1\n";
  out << "                  region_1\n";
  out << "                  region_2\n";
  out << "            zone_2\n";
  out << "                  region_1\n";
  out << "                  region_2\n";
  out << "\n";
  out << "           If you want to select all regions with name \"region_1\",\n";
  out << "       select the regex \"region_1\"\n";
  out << "\n";
  out << "           If you want to select only region \"mesh/zone_1/region_1\"\n";
  out << "       select or instance the regex \"zone_1/region_1\"\n";
  
  
  return out.str();
}
  
/////////////////////////////////////////////////////////////////////////////

void CMeshExtract::transform(const CMesh::Ptr& mesh, const std::vector<std::string>& args)
{

  m_mesh = mesh;

  
  // Storage of regions to keep
  std::list<std::string> keep_region_paths;

  
  // special cases "volumes" and "surfaces" as arg
  BOOST_FOREACH(const std::string region_name, args)
  {
    if (boost::regex_match(region_name,boost::regex("[Ss]urface(s)?")))   // Surface, Surfaces, surface, surfaces
    {
      BOOST_FOREACH( const CElements& elements, recursive_filtered_range_typed<CElements>(*m_mesh,IsElementsSurface()))
      {
        keep_region_paths.push_back(elements.get_parent()->full_path().string());
      }
    }
    else if (boost::regex_match(region_name,boost::regex("[Vv]olume(s)?"))) // Volume, Volumes, volume, volumes
    {
      BOOST_FOREACH( const CElements& elements, recursive_filtered_range_typed<CElements>(*m_mesh,IsElementsVolume()))
      {
        keep_region_paths.push_back(elements.get_parent()->full_path().string());
      }
    }
  }
  
  // For every region, see if its path matches the regex, and store it in a list
  BOOST_FOREACH( CRegion& region, recursive_range_typed<CRegion>(*m_mesh))
  {
    // see if this region has to be deleted.
    bool found = false;
    BOOST_FOREACH(const std::string& expression, args)
    {
      if (boost::regex_match(region.full_path().string(),boost::regex(".*"+expression+".*")))
      {
        found = true;
        keep_region_paths.push_back(region.full_path().string());
        break;
      }
    }
  }
  
  // Parse the list into individual regions that will not be removed
  std::set<std::string> keep_region;
  BOOST_FOREACH(const boost::filesystem::path& region_path, keep_region_paths)
  {
    BOOST_FOREACH(const std::string& region_name, region_path)
      keep_region.insert(region_name);
  }

  // Remove regions whose name doesn't appear in the parsed list "keep_region"
  BOOST_FOREACH( CRegion& region, recursive_range_typed<CRegion>(*m_mesh))
  {
    bool found = (std::find(keep_region.begin(),keep_region.end(),region.name()) != keep_region.end());
    if (!found)  region.get_parent()->remove_component(region.name());
  }
  
  
  // remove regions that have no elements
  BOOST_FOREACH( CRegion& region, recursive_range_typed<CRegion>(*m_mesh))
  {
    if (region.recursive_elements_count() == 0)
    {
      CFinfo << "removing empty element_region " << region.full_path().string() << CFendl;
      region.get_parent()->remove_component(region.name());
    }
  }
  
}

//////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
