// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/algorithm/string.hpp>

#include "Common/Foreach.hpp"
#include "Common/CGroup.hpp"
#include "Common/CLink.hpp"
#include "Common/CBuilder.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CList.hpp"
#include "Mesh/CDynTable.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CMesh.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ComponentBuilder < CRegion, Component, LibMesh > CRegion_Builder;


////////////////////////////////////////////////////////////////////////////////

CRegion::CRegion ( const std::string& name  ) :
  Component ( name )
{
   
}

////////////////////////////////////////////////////////////////////////////////

CRegion::~CRegion()
{
}

////////////////////////////////////////////////////////////////////////////////

CRegion& CRegion::create_region( const std::string& name, bool ensure_unique )
{
  if (ensure_unique)
  {
    return *create_component<CRegion>(name);
  }
  else
  {
    CRegion::Ptr region = get_child<CRegion>(name);
    if (!region)
      region = create_component<CRegion>(name);
    
    return *region;
  }
}

////////////////////////////////////////////////////////////////////////////////

CElements& CRegion::create_elements(const std::string& element_type_name, CNodes& nodes)
{
  std::string name = "elements_" + element_type_name;
  
  CElements::Ptr elements = get_child<CElements>(name);
  if (!elements)
  {
    elements = create_component<CElements>(name);
    elements->add_tag("GeometryElements");
    elements->initialize(element_type_name,nodes);
  }
  return *elements;
}

//////////////////////////////////////////////////////////////////////////////

CNodes& CRegion::create_nodes(const Uint& dim)
{  
  ///@todo nodes have to be created in CMesh
  CNodes::Ptr nodes = find_component_ptr_with_tag<CNodes>(*this,"nodes");
  if ( is_null(nodes) )
  {
    nodes = create_component<CNodes>("nodes");
    nodes->coordinates().set_row_size(dim);
    
    ///@todo when nodes in CMesh created, this can be linked inside CMesh
    find_component_with_name<CLink>(find_parent_component<CMesh>(*this),"nodes").link_to(nodes);
  }
  cf_assert(nodes->coordinates().row_size() == dim);

  return *nodes;
}

//////////////////////////////////////////////////////////////////////////////

void CRegion::add_field_link(CField& field)
{
  CGroup::Ptr field_group = get_child<CGroup>("fields");
  if (!field_group.get())
    field_group = create_component<CGroup>("fields");
  field_group->create_component<CLink>(field.field_name())->link_to(field.follow());
}

//////////////////////////////////////////////////////////////////////////////

CField& CRegion::get_field(const std::string& field_name)
{
  Component::Ptr all_fields = get_child("fields");
  cf_assert(all_fields.get());
  Component::Ptr field = all_fields->get_child(field_name);
  cf_assert(field.get());
  return *field->follow()->as_type<CField>();
}

//////////////////////////////////////////////////////////////////////////////

Uint CRegion::recursive_elements_count() const
{
  Uint elem_count = 0;
  boost_foreach (const CElements& elements, elements_range() )
    elem_count += elements.size();
  return elem_count;
}
  
//////////////////////////////////////////////////////////////////////////////

Uint CRegion::recursive_nodes_count() const
{
  std::set<Uint> nodes;
  boost_foreach (const CElements& elements, find_components_recursively<CElements>(*this))
  {
    boost_foreach (const Uint node, elements.used_nodes().array())
      nodes.insert(node);
  }
  return nodes.size();
}

////////////////////////////////////////////////////////////////////////////////

const CRegion& CRegion::subregion(const std::string& name) const
{
  return find_component_with_name<CRegion const>(*this,name);
}

////////////////////////////////////////////////////////////////////////////////

CRegion& CRegion::subregion(const std::string& name)
{
  return find_component_with_name<CRegion>(*this,name);
}

////////////////////////////////////////////////////////////////////////////////

const CElements& CRegion::elements(const std::string& name) const
{
  return find_component_with_name<CElements const>(*this,name);
}

////////////////////////////////////////////////////////////////////////////////

CElements& CRegion::elements(const std::string& name)
{
  return find_component_with_name<CElements>(*this,name);
}

////////////////////////////////////////////////////////////////////////////////

const CNodes& CRegion::nodes() const
{
  return find_parent_component<CMesh>(*this).nodes();
}

////////////////////////////////////////////////////////////////////////////////

CNodes& CRegion::nodes()
{
  return find_parent_component<CMesh>(*this).nodes();
}

////////////////////////////////////////////////////////////////////////////////

CRegion::ConstElementsRange CRegion::elements_range() const
{ 
  return find_components_recursively<CElements>(*this); 
}

////////////////////////////////////////////////////////////////////////////////

CRegion::ElementsRange CRegion::elements_range()
{ 
  return find_components_recursively<CElements>(*this); 
}

//////////////////////////////////////////////////////////////////////////////


} // Mesh
} // CF
