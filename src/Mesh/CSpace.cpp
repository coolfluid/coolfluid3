// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CreateComponent.hpp"
#include "Common/FindComponents.hpp"
#include "Common/CBuilder.hpp"

#include "Mesh/CSpace.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CEntities.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ComponentBuilder < CSpace, Component, LibMesh > CSpace_Builder;

////////////////////////////////////////////////////////////////////////////////

CSpace::CSpace ( const std::string& name ) :
  Component ( name )
{
  properties()["brief"] = std::string("Spaces are other views of CEntities, for instance a higher-order representation");
//  properties()["description"] = std::string("");

  mark_basic();

}

////////////////////////////////////////////////////////////////////////////////

CSpace::~CSpace()
{
}

////////////////////////////////////////////////////////////////////////////////

void CSpace::initialize(const std::string& shape_function_builder_name)
{
  m_shape_function = create_component_abstract_type<ElementType>( shape_function_builder_name, shape_function_builder_name );
  m_shape_function->rename(m_shape_function->element_type_name());
  add_static_component( m_shape_function );
  
  m_node_connectivity = create_static_component<CConnectivity>(Mesh::Tags::connectivity_table());
  m_node_connectivity->add_tag(Mesh::Tags::connectivity_table());
  m_node_connectivity->properties()["brief"] = std::string("The connectivity table specifying for each element the nodes in the coordinates table");
  
  /// @todo Now the connectivity table is being built from the nodes of the support
  /// without extra nodes... 
  CEntities& support = find_parent_component<CEntities>(*this);
  CConnectivity& ctable = node_connectivity();
  ctable.set_row_size( support.element_type().nb_nodes());
  Uint nb_elems = support.size();
  ctable.resize(nb_elems);
  for (Uint i=0; i<nb_elems; ++i)
  {
    index_foreach(j,const Uint node, support.get_nodes(i))
    {
      ctable[i][j]=node;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void CSpace::initialize(const CElements& elements)
{
  // Now the shape_function and connectivity_table are copies of the connectivity of the support
  CElements& support = find_parent_component<CElements>(*this);
  m_shape_function = support.get_child_ptr(support.element_type().name())->as_ptr<ElementType>();
  m_node_connectivity = support.node_connectivity().as_ptr<CConnectivity>();
}

////////////////////////////////////////////////////////////////////////////////

CConnectivity::ConstRow CSpace::get_nodes(const Uint elem_idx)
{
  return node_connectivity()[elem_idx];
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
