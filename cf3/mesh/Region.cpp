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
#include "mesh/Dictionary.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Faces.hpp"

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
  return *create_component<Region>(name);
}

////////////////////////////////////////////////////////////////////////////////

Elements& Region::create_elements(const std::string& element_type_name, Dictionary& nodes)
{
  std::string name = "elements_" + element_type_name;

  Handle< Component > celems = get_child(name);
  if ( is_null(celems) )
  {
    Handle<Elements> elements;

    boost::shared_ptr<ElementType> elem_type = boost::dynamic_pointer_cast<ElementType>(build_component( element_type_name, element_type_name ));

    if(is_null(elem_type))
      throw BadPointer(FromHere(),"");

    if (elem_type->dimensionality() == elem_type->dimension())
      elements = create_component<Cells>(name);
    else if (elem_type->dimensionality() == elem_type->dimension()-1)
      elements = create_component<Faces>(name);
    else
      throw NotImplemented(FromHere(), "Edge elements are not implemented yet");

    elements->initialize(element_type_name,nodes);

    return *elements;
  }
  else
    return dynamic_cast<Elements&>(*celems);
}

//////////////////////////////////////////////////////////////////////////////

Uint Region::global_elements_count(bool include_ghost_elems) const
{
  Uint elem_count = 0;
  if(include_ghost_elems)
  {
    boost_foreach (const Entities& elements, elements_range() )
      elem_count += elements.glb_size();
  }
  else
  {
    throw NotImplemented(FromHere(),"TODO");
  }
  return elem_count;
}

//////////////////////////////////////////////////////////////////////////////

Uint Region::recursive_elements_count(bool include_ghost_elems) const
{
  Uint elem_count = 0;
  if(include_ghost_elems)
  {
    boost_foreach (const Entities& elements, elements_range() )
      elem_count += elements.size();
  }
  else
  {
    Uint nb_ghost;
    boost_foreach (const Entities& elements, elements_range() )
    {
      elem_count += elements.size();

      // Count ghosts elements and subtract
      nb_ghost = 0;
      for (Uint e=0; e<elements.size(); ++e)
        if (elements.is_ghost(e))
          ++nb_ghost;
      elem_count -= nb_ghost;
    }
  }
  return elem_count;
}

//////////////////////////////////////////////////////////////////////////////

Uint Region::recursive_nodes_count()
{
  return Elements::used_nodes(*this).size();
}

//////////////////////////////////////////////////////////////////////////////

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
