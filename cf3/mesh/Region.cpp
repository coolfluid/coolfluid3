// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>

#include "common/Foreach.hpp"
#include "common/Group.hpp"
#include "common/Link.hpp"
#include "common/Builder.hpp"

#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "common/Table.hpp"
#include "common/Table.hpp"
#include "common/List.hpp"
#include "common/DynTable.hpp"
#include "mesh/SpaceFields.hpp"
#include "mesh/Mesh.hpp"

namespace cf3 {
namespace mesh {

using namespace common;

common::ComponentBuilder < Region, Component, LibMesh > Region_Builder;


////////////////////////////////////////////////////////////////////////////////

Region::Region ( const std::string& name  ) :
  Component ( name )
{
  mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

Region::~Region()
{
}

////////////////////////////////////////////////////////////////////////////////

Region& Region::create_region( const std::string& name )
{
  return create_component<Region>(name);
}

////////////////////////////////////////////////////////////////////////////////

Elements& Region::create_elements(const std::string& element_type_name, SpaceFields& nodes)
{
  std::string name = "elements_" + element_type_name;

  Component::Ptr celems = get_child_ptr(name);
  if ( is_null(celems) )
  {
    Elements& elements = create_elements(element_type_name);
    elements.assign_geometry(nodes);
    return elements;
  }
  else
    return celems->as_type<Elements>();
}

Elements& Region::create_elements(const std::string& element_type_name)
{
  std::string name = "elements_" + element_type_name;

  Component::Ptr celems = get_child_ptr(name);
  if ( is_null(celems) )
  {
    Elements::Ptr elements = create_component_ptr<Elements>(name);
    elements->add_tag("SpaceFieldsElements");
    elements->initialize(element_type_name);
    return *elements;
  }
  else
    return celems->as_type<Elements>();
}

//////////////////////////////////////////////////////////////////////////////

Uint Region::recursive_elements_count() const
{
  Uint elem_count = 0;
  boost_foreach (const Entities& elements, elements_range() )
    elem_count += elements.size();
  return elem_count;
}

//////////////////////////////////////////////////////////////////////////////

Uint Region::recursive_nodes_count()
{
  return Elements::used_nodes(*this).size();
}

////////////////////////////////////////////////////////////////////////////////

const Region& Region::subregion(const std::string& name) const
{
  return find_component_with_name<Region const>(*this,name);
}

////////////////////////////////////////////////////////////////////////////////

Region& Region::subregion(const std::string& name)
{
  return find_component_with_name<Region>(*this,name);
}

////////////////////////////////////////////////////////////////////////////////

const Elements& Region::elements(const std::string& name) const
{
  return find_component_with_name<Elements const>(*this,name);
}

////////////////////////////////////////////////////////////////////////////////

Elements& Region::elements(const std::string& name)
{
  return find_component_with_name<Elements>(*this,name);
}

////////////////////////////////////////////////////////////////////////////////

SpaceFields& Region::geometry_fields() const
{
  return find_parent_component<Mesh>(*this).geometry_fields();
}

////////////////////////////////////////////////////////////////////////////////

Region::ConstElementsRange Region::elements_range() const
{
  return find_components_recursively<Entities>(*this);
}

////////////////////////////////////////////////////////////////////////////////

Region::ElementsRange Region::elements_range()
{
  return find_components_recursively<Entities>(*this);
}

//////////////////////////////////////////////////////////////////////////////


} // mesh
} // cf3
