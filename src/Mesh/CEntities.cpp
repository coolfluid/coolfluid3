// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "Common/CLink.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/FindComponents.hpp"
#include "Common/StringConversion.hpp"
#include "Common/OptionT.hpp"

#include "Mesh/CEntities.hpp"
#include "Mesh/CList.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CSpace.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CEntities::CEntities ( const std::string& name ) :
  Component ( name )
{
  mark_basic();
  properties()["brief"] = std::string("Holds information of elements of one type");
  properties()["description"] = std::string("Container component that stores the element to node connectivity,\n")
  +std::string("a link to node storage, a list of used nodes, and global numbering unique over all processors");

  properties().add_option(OptionT<std::string>::create("element_type","Element Type","Element type", std::string("")))
    ->attach_trigger(boost::bind(&CEntities::configure_element_type, this));

  m_global_numbering = create_static_component<CList<Uint> >(Mesh::Tags::global_elem_indices());
  m_global_numbering->add_tag(Mesh::Tags::global_elem_indices());
  m_global_numbering->properties()["brief"] = std::string("The global element indices (inter processor)");

  m_nodes = create_static_component<CLink>(Mesh::Tags::nodes());
  m_nodes->add_tag(Mesh::Tags::nodes());
}

////////////////////////////////////////////////////////////////////////////////

CEntities::~CEntities()
{
}

//////////////////////////////////////////////////////////////////////////////

void CEntities::initialize(const std::string& element_type_name, CNodes& nodes)
{
  m_nodes->link_to(nodes.follow());
  configure_property("element_type",element_type_name);
  cf_assert(is_not_null(m_element_type));
}

////////////////////////////////////////////////////////////////////////////////

void CEntities::configure_element_type()
{
  const std::string etype_name = property("element_type").value<std::string>();
  if (is_not_null(m_element_type))
  {
    remove_component(m_element_type->name());
  }
  m_element_type = create_component_abstract_type<ElementType>( etype_name, etype_name );
  m_element_type->rename(m_element_type->element_type_name());
  add_component( m_element_type );
}

//////////////////////////////////////////////////////////////////////////////

const ElementType& CEntities::element_type() const
{
  cf_assert_desc("element_type not initialized", is_not_null(m_element_type));
  return *m_element_type;
}

//////////////////////////////////////////////////////////////////////////////

const CNodes& CEntities::nodes() const
{
  return m_nodes->follow()->as_type<CNodes>();
}

////////////////////////////////////////////////////////////////////////////////

CNodes& CEntities::nodes()
{
  return m_nodes->follow()->as_type<CNodes>();
}

////////////////////////////////////////////////////////////////////////////////

CList<Uint>& CEntities::used_nodes(Component& parent)
{
  CList<Uint>::Ptr used_nodes = find_component_ptr_with_tag<CList<Uint> >(parent,Mesh::Tags::nodes_used());
  if (is_null(used_nodes))
  {
    used_nodes = parent.create_component<CList<Uint> >(Mesh::Tags::nodes_used());
    used_nodes->add_tag(Mesh::Tags::nodes_used());
    used_nodes->properties()["brief"] = std::string("The local node indices used by the parent component");

    // Assemble all unique node numbers in a set
    std::set<Uint> node_set;

    if ( CEntities::Ptr elements = parent.as_ptr<CEntities>() )
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

CSpace& CEntities::create_space( const std::string& shape_function_builder_name )
{
  Uint nb_existing_spaces = m_spaces.size();
  CSpace::Ptr space = create_component<CSpace>("space["+to_str(nb_existing_spaces)+"]");
  space->initialize(shape_function_builder_name);
  m_spaces.push_back(space);
  return *space;
}

////////////////////////////////////////////////////////////////////////////////

CSpace& CEntities::create_space0()
{
  if (m_spaces.size() == 0)
  {
    CSpace& space = create_space(element_type().builder_name());
    CConnectivity& table = space.node_connectivity();
    table.set_row_size(space.shape_function().nb_nodes());
    table.resize(size());
    for (Uint i=0; i!=table.size(); ++i)
    {
      CTable<Uint>::ConstRow nodes = get_nodes(i);
      for (Uint j=0; j!=table.row_size(); ++j)
        table[i][j] = nodes[j];
    }
  }
  return *m_spaces[0];
}

////////////////////////////////////////////////////////////////////////////////

const CSpace& CEntities::space (const Uint space_idx) const
{
  return *m_spaces[space_idx];
}

////////////////////////////////////////////////////////////////////////////////

bool CEntities::exists_space(const Uint space_idx) const
{
  bool exists = false;
  if (m_spaces.size() > space_idx)
    if ( is_not_null (m_spaces[space_idx]) )
      exists = true;
  return exists;
}

////////////////////////////////////////////////////////////////////////////////
} // Mesh
} // CF
