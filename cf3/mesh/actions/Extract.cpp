// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
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
  options().add("regions", std::vector<std::string>())
      .description("Regions to extract, can be regular expression matched with the full path")
      .mark_basic();


  properties()["brief"] = std::string("Extract given regions from the mesh");
  std::stringstream desc;
  desc << "  Usage: Extract [region_name1 region_name2 ...]\n\n";
  desc << "          Special cases: \"surfaces\", \"volumes\" as region_name.\n";
  desc << "      A region_name can be a regular expression matched with the full path.\n";
  desc << "  Example:\n";
  desc << "          Given a mesh with data organized in the following way:\n";
  desc << "      mesh\n";
  desc << "            zone_1\n";
  desc << "                  region_1\n";
  desc << "                  region_2\n";
  desc << "            zone_2\n";
  desc << "                  region_1\n";
  desc << "                  region_2\n";
  desc << "\n";
  desc << "           If you want to select all regions with name \"region_1\",\n";
  desc << "       select the regex \"region_1\"\n";
  desc << "\n";
  desc << "           If you want to select only region \"mesh/zone_1/region_1\"\n";
  desc << "       select or instance the regex \"zone_1/region_1\"\n";
  properties()["description"]=desc.str();
}

/////////////////////////////////////////////////////////////////////////////

void Extract::execute()
{

  Mesh& mesh = *m_mesh;


  // Storage of regions to keep
  std::set<std::string> keep_region_paths;

  std::vector<std::string> args = options().value< std::vector<std::string> >("regions");

  // special cases "volumes" and "surfaces" as arg
  BOOST_FOREACH(const std::string region_name, args)
  {
    if (boost::regex_match(region_name,boost::regex("[Ss]urface(s)?")))   // Surface, Surfaces, surface, surfaces
    {
      BOOST_FOREACH( const Elements& elements, find_components_recursively_with_filter<Elements>(mesh,IsElementsSurface()))
      {
        keep_region_paths.insert(elements.parent()->uri().path());
      }
    }
    else if (boost::regex_match(region_name,boost::regex("[Vv]olume(s)?"))) // Volume, Volumes, volume, volumes
    {
      BOOST_FOREACH( const Elements& elements, find_components_recursively_with_filter<Elements>(mesh,IsElementsVolume()))
      {
        keep_region_paths.insert(elements.parent()->uri().path());
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
        keep_region_paths.insert(region.uri().path());
        Handle<Component> parent = region.parent();
        while ( parent != mesh.handle() )
        {
          keep_region_paths.insert(parent->uri().path());
          parent = parent->parent();
        }
        break;
      }
    }
  }

  CFdebug << "regions to keep:" << CFendl;
  BOOST_FOREACH(const URI& region_path, keep_region_paths)
  {
    CFdebug << "  - " << region_path << CFendl;
  }

  // Remove regions whose name doesn't appear in the parsed list "keep_region"
  BOOST_FOREACH( Region& region, find_components_recursively<Region>(mesh))
  {
    bool found = (std::find(keep_region_paths.begin(),keep_region_paths.end(),region.uri().path()) != keep_region_paths.end());
    if (!found)
    {
      if (region.parent())
        region.parent()->remove_component(region.name());
    }
  }


}

//////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3
