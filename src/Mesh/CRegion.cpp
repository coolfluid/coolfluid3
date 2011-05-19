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
  mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

CRegion::~CRegion()
{
}

////////////////////////////////////////////////////////////////////////////////

CRegion& CRegion::create_region( const std::string& name )
{
  return create_component<CRegion>(name);
}

////////////////////////////////////////////////////////////////////////////////

CElements& CRegion::create_elements(const std::string& element_type_name, CNodes& nodes)
{
  std::string name = "elements_" + element_type_name;

  Component::Ptr celems = get_child_ptr(name);
  if ( is_null(celems) )
  {
    CElements::Ptr elements = create_component_ptr<CElements>(name);
    elements->add_tag("GeometryElements");
    elements->initialize(element_type_name,nodes);
    return *elements;
  }
  else
    return celems->as_type<CElements>();
}

//////////////////////////////////////////////////////////////////////////////

CNodes& CRegion::create_nodes(const Uint& dim)
{
  /// @todo nodes have to be created in CMesh
  CNodes::Ptr nodes = find_component_ptr_with_tag<CNodes>(*this,Mesh::Tags::nodes());
  if ( is_null(nodes) )
  {
    nodes = create_component_ptr<CNodes>(Mesh::Tags::nodes());
    nodes->coordinates().set_row_size(dim);

    /// @todo when nodes in CMesh created, this can be linked inside CMesh
    find_component_with_name<CLink>(find_parent_component<CMesh>(*this),Mesh::Tags::nodes()).link_to(nodes);
  }
  cf_assert(nodes->coordinates().row_size() == dim);

  return *nodes;
}

//////////////////////////////////////////////////////////////////////////////

Uint CRegion::recursive_elements_count() const
{
  Uint elem_count = 0;
  boost_foreach (const CEntities& elements, elements_range() )
    elem_count += elements.size();
  return elem_count;
}

//////////////////////////////////////////////////////////////////////////////

Uint CRegion::recursive_nodes_count()
{
  return CElements::used_nodes(*this).size();
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
  return find_components_recursively<CEntities>(*this);
}

////////////////////////////////////////////////////////////////////////////////

CRegion::ElementsRange CRegion::elements_range()
{
  return find_components_recursively<CEntities>(*this);
}

//////////////////////////////////////////////////////////////////////////////


} // Mesh
} // CF
