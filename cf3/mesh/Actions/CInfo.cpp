// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/CBuilder.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/StringConversion.hpp"

#include "mesh/CElements.hpp"
#include "mesh/CRegion.hpp"
#include "mesh/Field.hpp"
#include "mesh/CMesh.hpp"

#include "mesh/Actions/CInfo.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace Actions {

  using namespace common;

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

common::ComponentBuilder < CInfo, CMeshTransformer, LibActions> CInfo_Builder;

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
  return properties().value<std::string>("brief");
}

/////////////////////////////////////////////////////////////////////////////


std::string CInfo::help() const
{
  return "  " + properties().value<std::string>("brief") + "\n" +
      properties().value<std::string>("description");
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
  boost_foreach( const Field& field, find_components<Field>(mesh) )
  {
    CFinfo << " - " << field.name() << "  (" << FieldGroup::Basis::Convert::instance().to_str(field.basis()) << ")" << CFendl;
    for (Uint i=0; i<field.nb_vars(); ++i)
    {
      CFinfo << "     " << field.var_name(i) << "[" << (Uint) field.var_length(i) << "]" << CFendl;
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
} // mesh
} // cf3
