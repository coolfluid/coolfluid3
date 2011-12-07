// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include <boost/foreach.hpp>
#include <boost/regex.hpp>

#include "common/BoostFilesystem.hpp"

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

#include "mesh/Elements.hpp"
#include "mesh/Region.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"

#include "mesh/actions/Extract.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions{

  using namespace common;

  class IsGroup
  {
  public:
    IsGroup () {}

    bool operator()(const Component& component)
    {
      return count(find_components<Elements>(component));
    }
  }; // IsGroup


////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < mesh::actions::Extract, mesh::MeshTransformer, mesh::actions::LibActions > Extract_Builder;

//////////////////////////////////////////////////////////////////////////////

Extract::Extract( const std::string& name )
: MeshTransformer(name)
{
  options().add_option("Regions", std::vector<std::string>())
      .description("Regions to extract, can be regular expression matched with the full path")
      .mark_basic();
}

/////////////////////////////////////////////////////////////////////////////

std::string Extract::brief_description() const
{
  return "Extract given regions from the mesh";
}

/////////////////////////////////////////////////////////////////////////////

std::string Extract::help() const
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

void Extract::execute()
{

  Mesh& mesh = *m_mesh;


  // Storage of regions to keep
  std::list<std::string> keep_region_paths;

  std::vector<std::string> args = options().option("Regions").value< std::vector<std::string> >();

  // special cases "volumes" and "surfaces" as arg
  BOOST_FOREACH(const std::string region_name, args)
  {
    if (boost::regex_match(region_name,boost::regex("[Ss]urface(s)?")))   // Surface, Surfaces, surface, surfaces
    {
      BOOST_FOREACH( const Elements& elements, find_components_recursively_with_filter<Elements>(mesh,IsElementsSurface()))
      {
        keep_region_paths.push_back(elements.parent()->uri().path());
      }
    }
    else if (boost::regex_match(region_name,boost::regex("[Vv]olume(s)?"))) // Volume, Volumes, volume, volumes
    {
      BOOST_FOREACH( const Elements& elements, find_components_recursively_with_filter<Elements>(mesh,IsElementsVolume()))
      {
        keep_region_paths.push_back(elements.parent()->uri().path());
      }
    }
  }

  // For every region, see if its path matches the regex, and store it in a list
  BOOST_FOREACH( Region& region, find_components_recursively<Region>(mesh))
  {
    // see if this region has to be deleted.
    bool found = false;
    BOOST_FOREACH(const std::string& expression, args)
    {
      if (boost::regex_match(region.uri().path(),boost::regex(".*"+expression+".*")))
      {
        found = true;
        keep_region_paths.push_back(region.uri().path());
        break;
      }
    }
  }

  // Parse the list into individual regions that will not be removed
  std::set<std::string> keep_region;
  BOOST_FOREACH(const URI& region_path, keep_region_paths)
  {
    keep_region.insert(region_path.name());
  }

  // Remove regions whose name doesn't appear in the parsed list "keep_region"
  BOOST_FOREACH( Region& region, find_components_recursively<Region>(mesh))
  {
    bool found = (std::find(keep_region.begin(),keep_region.end(),region.name()) != keep_region.end());
    if (!found)  region.parent()->remove_component(region.name());
  }


  // remove regions that have no elements
  BOOST_FOREACH( Region& region, find_components_recursively<Region>(mesh))
  {
    if (region.recursive_elements_count() == 0)
    {
      CFinfo << "removing empty element_region " << region.uri().string() << CFendl;
      region.parent()->remove_component(region.name());
    }
  }

}

//////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3
