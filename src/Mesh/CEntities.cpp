// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "Common/CLink.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CEntities.hpp"
#include "Mesh/CList.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/ElementType.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CEntities::CEntities ( const std::string& name ) :
  Component ( name )
{
  properties()["brief"] = std::string("Holds information of elements of one type");
  properties()["description"] = std::string("Container component that stores the element to node connectivity,\n")
  +std::string("a link to node storage, a list of used nodes, and global numbering unique over all processors");
  
  
  m_global_numbering = create_static_component<CList<Uint> >("global_element_indices");
  m_global_numbering->add_tag("global_element_indices");
  m_global_numbering->properties()["brief"] = std::string("The global element indices (inter processor)");

  m_nodes = create_static_component<CLink>("nodes");
  m_nodes->add_tag("nodes");
  
}

////////////////////////////////////////////////////////////////////////////////

CEntities::~CEntities()
{
}

//////////////////////////////////////////////////////////////////////////////

void CEntities::initialize(const std::string& element_type_name, CNodes& nodes)
{
  m_nodes->link_to(nodes.follow());

  set_element_type(element_type_name);
}

////////////////////////////////////////////////////////////////////////////////

void CEntities::set_element_type(const std::string& etype_name)
{
  m_element_type = create_component_abstract_type<ElementType>( etype_name, etype_name );
  m_element_type->rename(m_element_type->element_type_name());
  add_static_component( m_element_type );
}

//////////////////////////////////////////////////////////////////////////////

const ElementType& CEntities::element_type() const 
{ 
  cf_assert_desc("element_type not initialized", is_not_null(m_element_type));
  return *m_element_type; 
}

//////////////////////////////////////////////////////////////////////////////

CNodes& CEntities::nodes()
{
  return *m_nodes->follow()->as_type<CNodes>();
}

//////////////////////////////////////////////////////////////////////////////

const CNodes& CEntities::nodes() const
{
  return *m_nodes->follow()->as_type<CNodes>();
}

//////////////////////////////////////////////////////////////////////////////

CList<Uint>& CEntities::used_nodes(Component& parent)
{
  CList<Uint>::Ptr used_nodes = find_component_ptr_with_tag<CList<Uint> >(parent,"used_nodes");
  if (is_null(used_nodes))
  {
    used_nodes = parent.create_component<CList<Uint> >("used_nodes");
    used_nodes->add_tag("used_nodes");
    used_nodes->properties()["brief"] = std::string("The local node indices used by the parent component");
    
    // Assemble all unique node numbers in a set
    std::set<Uint> node_set;
    
    if ( CEntities::Ptr elements = parent.as_type<CEntities>() )
    {
      const Uint nb_elems = elements->size();
      for (Uint idx=0; idx<nb_elems; ++idx)
      {
        boost_foreach(const Uint node, elements->get_nodes(idx))
        {
          node_set.insert(node);
        }
      }
    }
    else
    {
      boost_foreach(CEntities& elements, find_components_recursively<CEntities>(parent))
      {
        const Uint nb_elems = elements.size();
        for (Uint idx=0; idx<nb_elems; ++idx)
        {
          boost_foreach(const Uint node, elements.get_nodes(idx))
          {
            node_set.insert(node);
          }
        }
      }
    }
    
    // Copy the set to the node_list
    used_nodes->resize(node_set.size());
    CList<Uint>::ListT& nodes_array = used_nodes->array();
    index_foreach(i,const Uint node, node_set)
      nodes_array[i] = node;
  }
  return *used_nodes;
}

////////////////////////////////////////////////////////////////////////////////

Uint CEntities::size() const
{
  throw ShouldNotBeHere( FromHere(), " This virtual function has to be overloaded. ");
}

CTable<Uint>::ConstRow CEntities::get_nodes(const Uint elem_idx)
{
  throw ShouldNotBeHere( FromHere(), " This virtual function has to be overloaded. ");
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
