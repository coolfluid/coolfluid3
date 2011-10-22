// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include <boost/assign/list_of.hpp>
#include "common/Link.hpp"

#include "common/FindComponents.hpp"
#include "common/StringConversion.hpp"
#include "common/OptionT.hpp"
#include "common/Signal.hpp"
#include "common/XML/SignalOptions.hpp"
#include "common/PE/Comm.hpp"

#include "mesh/Connectivity.hpp"
#include "mesh/List.hpp"
#include "mesh/Geometry.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/Space.hpp"

namespace cf3 {
namespace mesh {

using namespace common;

////////////////////////////////////////////////////////////////////////////////

Entities::Entities ( const std::string& name ) :
  Component ( name )
{
  mark_basic();
  properties()["brief"] = std::string("Holds information of elements of one type");
  properties()["description"] = std::string("Container component that stores the element to node connectivity,\n")
  +std::string("a link to node storage, a list of used nodes, and global numbering unique over all processors");

  m_options.add_option(OptionT<std::string>::create("element_type", std::string("")))
      ->description("Element type")
      ->pretty_name("Element type")
      ->attach_trigger(boost::bind(&Entities::configure_element_type, this));

  m_global_numbering = create_static_component_ptr<List<Uint> >(mesh::Tags::global_elem_indices());
  m_global_numbering->add_tag(mesh::Tags::global_elem_indices());
  m_global_numbering->properties()["brief"] = std::string("The global element indices (inter processor)");

  m_spaces_group = create_static_component_ptr<Group>("spaces");
  m_spaces_group->mark_basic();

  m_rank = create_static_component_ptr< List<Uint> >("rank");
  m_rank->add_tag("rank");

  regist_signal ( "create_space" )
      ->connect ( boost::bind ( &Entities::signal_create_space, this, _1 ) )
      ->description( "Create space for other interpretations of fields (e.g. high order)" )
      ->pretty_name( "Create space" )
      ->signature(boost::bind(&Entities::signature_create_space, this, _1));

}

////////////////////////////////////////////////////////////////////////////////

Entities::~Entities()
{
}

//////////////////////////////////////////////////////////////////////////////

void Entities::initialize(const std::string& element_type_name)
{
  configure_option("element_type",element_type_name);
  cf3_assert(is_not_null(m_element_type));
}

void Entities::initialize(const std::string& element_type_name, Geometry& geometry)
{
  assign_geometry(geometry);
  initialize(element_type_name);
}

void Entities::assign_geometry(Geometry& geometry)
{
  m_geometry = geometry.as_ptr<Geometry>();
}

////////////////////////////////////////////////////////////////////////////////

void Entities::configure_element_type()
{
  const std::string etype_name = option("element_type").value<std::string>();
  if (is_not_null(m_element_type))
  {
    remove_component(m_element_type->name());
  }
  m_element_type = build_component_abstract_type<ElementType>( etype_name, etype_name );
  m_element_type->rename(m_element_type->derived_type_name());
  add_component( m_element_type );

  if ( exists_space(mesh::Tags::geometry()) )
  {
    space(mesh::Tags::geometry()).configure_option("shape_function",m_element_type->shape_function().derived_type_name());
  }
  else
  {
    Space& geometry_space = create_space(mesh::Tags::geometry(),element_type().shape_function().derived_type_name());
    geometry_space.add_tag(mesh::Tags::geometry());
    m_geometry_space = geometry_space.as_ptr<Space>();
  }
}

//////////////////////////////////////////////////////////////////////////////

ElementType& Entities::element_type() const
{
  cf3_assert_desc("element_type not initialized", is_not_null(m_element_type));
  return *m_element_type;
}

////////////////////////////////////////////////////////////////////////////////

List<Uint>& Entities::used_nodes(Component& parent, const bool rebuild)
{
  List<Uint>::Ptr used_nodes = find_component_ptr_with_tag<List<Uint> >(parent,mesh::Tags::nodes_used());
  if (rebuild && is_not_null(used_nodes))
  {
    parent.remove_component(*used_nodes);
    used_nodes.reset();
  }

  if (is_null(used_nodes))
  {
    used_nodes = parent.create_component_ptr<List<Uint> >(mesh::Tags::nodes_used());
    used_nodes->add_tag(mesh::Tags::nodes_used());
    used_nodes->properties()["brief"] = std::string("The local node indices used by the parent component");

    // Assemble all unique node numbers in a set
    std::set<Uint> node_set;

    if ( Entities::Ptr elements = parent.as_ptr<Entities>() )
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
      boost_foreach(Entities& elements, find_components_recursively<Entities>(parent))
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

    List<Uint>::ListT& nodes_array = used_nodes->array();
    Uint cnt=0;
    boost_foreach(const Uint node, node_set)
      nodes_array[cnt++] = node;


  }
  return *used_nodes;
}

////////////////////////////////////////////////////////////////////////////////

Uint Entities::size() const
{
  throw ShouldNotBeHere( FromHere(), " This virtual function has to be overloaded. ");
}

Table<Uint>::ConstRow Entities::get_nodes(const Uint elem_idx) const
{
  throw ShouldNotBeHere( FromHere(), " This virtual function has to be overloaded. ");
}

////////////////////////////////////////////////////////////////////////////////

Space& Entities::create_space( const std::string& name, const std::string& shape_function_builder_name )
{
  Space::Ptr space = m_spaces_group->create_component_ptr<Space>(name);
  space->configure_option("shape_function",shape_function_builder_name);
  space->set_support(*this);
  return *space;
}

////////////////////////////////////////////////////////////////////////////////

Space& Entities::space (const std::string& space_name) const
{
  return m_spaces_group->get_child(space_name).as_type<Space>();
}

////////////////////////////////////////////////////////////////////////////////

bool Entities::exists_space(const std::string& name) const
{
  return is_not_null(m_spaces_group->get_child_ptr(name));
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix Entities::get_coordinates(const Uint elem_idx) const
{
  throw common::NotImplemented(FromHere(),"Should implement in derived class");
  return RealMatrix(1,1);
}

////////////////////////////////////////////////////////////////////////////////

void Entities::put_coordinates(RealMatrix& coordinates, const Uint elem_idx) const
{
  throw common::NotImplemented(FromHere(),"Should implement in derived class");
}

////////////////////////////////////////////////////////////////////////////////

void Entities::allocate_coordinates(RealMatrix& coords) const
{
  coords.resize(element_type().nb_nodes(),element_type().dimension());
}

////////////////////////////////////////////////////////////////////////////////

void Entities::signature_create_space ( SignalArgs& node)
{
  XML::SignalOptions options( node );
  options.add_option< OptionT<std::string> >("name" , std::string("new_space") )
      ->description("Name to add to space");

  options.add_option< OptionT<std::string> >("shape_function" , std::string("cf3.mesh.LagrangeP0.Line") )
      ->description("Shape Function to add as space");
}

////////////////////////////////////////////////////////////////////////////////

void Entities::signal_create_space ( SignalArgs& node )
{
  XML::SignalOptions options( node );

  std::string name = options.value<std::string>("name");

  std::string shape_function_builder = options.value<std::string>("shape_function");

  Space& space = create_space(name, shape_function_builder);
}

////////////////////////////////////////////////////////////////////////////////

bool Entities::is_ghost(const Uint idx) const
{
  cf3_assert_desc(to_str(idx)+">="+to_str(size()),idx < size());
  cf3_assert(size() == m_rank->size());
  cf3_assert(idx<m_rank->size());
  return (*m_rank)[idx] != PE::Comm::instance().rank();
}

////////////////////////////////////////////////////////////////////////////////

bool IsElementsVolume::operator()(const Entities::Ptr& component)
{
  return component->element_type().dimension() == component->element_type().dimensionality();
}

bool IsElementsVolume::operator()(const Entities& component)
{
  return component.element_type().dimension() == component.element_type().dimensionality();
}

bool IsElementsSurface::operator()(const Entities::Ptr& component)
{
  return component->element_type().dimension() == component->element_type().dimensionality() + 1;
}

bool IsElementsSurface::operator()(const Entities& component)
{
  return component.element_type().dimension() == component.element_type().dimensionality() + 1;
}

////////////////////////////////////////////////////////////////////////////////
} // mesh
} // cf3
