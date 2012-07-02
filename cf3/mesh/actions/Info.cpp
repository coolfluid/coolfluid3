// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/StringConversion.hpp"
#include "common/PropertyList.hpp"

#include "mesh/Elements.hpp"
#include "mesh/Region.hpp"
#include "mesh/Field.hpp"
#include "mesh/Mesh.hpp"

#include "mesh/actions/Info.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

  using namespace common;

  class IsElementRegion
  {
  public:
    IsElementRegion () {}

    bool operator()(const Component& component)
    {
      return !find_components<common::Table<Uint> >(component).empty() && !find_components<Entities>(component).empty();
    }

  }; // IsElementRegion

  class IsGroup
  {
  public:
    IsGroup () {}

    bool operator()(const Component& component)
    {
      return count(find_components_with_filter<Region>(component,m_isElement));
    }

  private:
    IsElementRegion m_isElement;
  }; // IsGroup

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Info, MeshTransformer, mesh::actions::LibActions> Info_Builder;

//////////////////////////////////////////////////////////////////////////////

Info::Info( const std::string& name )
: MeshTransformer(name)
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

void Info::execute()
{

  Mesh& mesh = *m_mesh;

  CFinfo << "Element distribution:" << CFendl;
  boost_foreach( const Region& region, find_components<Region>(mesh))
  {
    CFinfo << print_region_tree(region) << CFflush;
  }

  CFinfo << "Fields:" << CFendl;
  boost_foreach( const Dictionary& dict, find_components_recursively<Dictionary>(mesh))
  {
    CFinfo << "  " << dict.name() << "  ("<<dict.size()<<", " << (dict.continuous()? "continuous" : "discontinuous" ) << ")" << CFendl;
    boost_foreach( const Field& field, find_components<Field>(dict) )
    {
      CFinfo << "      " << field.name() << "["<<field.row_size() << "]" << CFendl;
      for (Uint i=0; i<field.nb_vars(); ++i)
      {
        CFinfo << "        - " << field.var_name(i) << "[" << (Uint) field.var_length(i) << "]" << CFendl;
      }
    }

  }

}

//////////////////////////////////////////////////////////////////////////////

std::string Info::print_region_tree(const Region& region, Uint level)
{
  std::string tree;

  for (Uint i=0; i<level; i++)
    tree += "    ";
  tree += "  " + region.name() + " (" + to_str(region.recursive_elements_count(true)) +  ")\n";

  tree += print_elements(region,level+1);

  boost_foreach( const Region& subregion, find_components_with_filter<Region>(region,IsComponentTrue()))
  {
    tree += print_region_tree(subregion,level+1);
  }
  return tree;
}

//////////////////////////////////////////////////////////////////////////////

std::string Info::print_elements(const Component& region, Uint level)
{
  std::string tree;
  boost_foreach( const Entities& elements_region, find_components<Entities>(region))
  {
    for (Uint i=0; i<level; i++)
      tree += "    ";
    std::string dimensionality = elements_region.element_type().dimension() == elements_region.element_type().dimensionality() ? "volume" : "surface";
    tree += elements_region.name() + " -- " + dimensionality + "  (" + to_str(elements_region.size()) +  ")\n";
  }
  return tree;
}

//////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3
