// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <ios>

#include <boost/assign/list_of.hpp>

#include "common/Link.hpp"
#include "common/FindComponents.hpp"
#include "common/StringConversion.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/Signal.hpp"
#include "common/List.hpp"
#include "common/PropertyList.hpp"

#include "common/XML/SignalOptions.hpp"

#include "common/PE/Comm.hpp"

#include "mesh/Connectivity.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/Space.hpp"
#include "mesh/Entities.hpp"
#include "mesh/Field.hpp"

namespace cf3 {
namespace mesh {

using namespace common;

RegistTypeInfo<Entity,LibMesh> regist_Entity;

////////////////////////////////////////////////////////////////////////////////

Entities::Entities ( const std::string& name ) :
  Component ( name )
{
  mark_basic();
  properties()["brief"] = std::string("Holds information of elements of one type");
  properties()["description"] = std::string("Container component that stores the element to node connectivity,\n")
  +std::string("a link to node storage, a list of used nodes, and global numbering unique over all processors");

  options().add_option("element_type", "")
      .description("Element type")
      .pretty_name("Element type")
      .attach_trigger(boost::bind(&Entities::configure_element_type, this)); 

  m_global_numbering = create_static_component<common::List<Uint> >(mesh::Tags::global_elem_indices());
  m_global_numbering->add_tag(mesh::Tags::global_elem_indices());
  m_global_numbering->properties()["brief"] = std::string("The global element indices (inter processor)");

  m_spaces_group = create_static_component<Group>("spaces");
  m_spaces_group->mark_basic();

  m_rank = create_static_component< common::List<Uint> >("rank");
  m_rank->add_tag("rank");

//  regist_signal ( "create_space" )
//      .connect ( boost::bind ( &Entities::signal_create_space, this, _1 ) )
//      .description( "Create space for other interpretations of fields (e.g. high order)" )
//      .pretty_name( "Create space" )
//      .signature(boost::bind(&Entities::signature_create_space, this, _1));

}

////////////////////////////////////////////////////////////////////////////////

Entities::~Entities()
{
}

//////////////////////////////////////////////////////////////////////////////

void Entities::initialize(const std::string& element_type_name)
{
  options().configure_option("element_type",element_type_name);
  cf3_assert(is_not_null(m_element_type));
}

void Entities::initialize(const std::string& element_type_name, Dictionary& geometry)
{
  initialize(element_type_name);
  create_geometry_space(geometry);
}

void Entities::create_geometry_space(Dictionary& geometry)
{
  if ( is_null(m_element_type) )
    throw SetupError(FromHere(),"option 'element_type' needs to be configured first");

  m_geometry_fields = Handle<Dictionary>(geometry.handle<Component>());
  if ( exists_space(mesh::Tags::geometry()) )
  {
    space(mesh::Tags::geometry()).options().configure_option("shape_function",m_element_type->shape_function().derived_type_name());
  }
  else
  {
    Space& geometry_space = create_space(element_type().shape_function().derived_type_name(),*m_geometry_fields);
    geometry_space.add_tag(mesh::Tags::geometry());
    m_geometry_space = Handle<Space>(geometry_space.handle<Component>());
  }
}

////////////////////////////////////////////////////////////////////////////////

void Entities::configure_element_type()
{
  const std::string etype_name = options().option("element_type").value<std::string>();
  if (is_not_null(m_element_type))
  {
    remove_component(m_element_type->name());
  }
  m_element_type = Handle<ElementType>(create_component( etype_name, etype_name ) );
  m_element_type->rename(m_element_type->derived_type_name());
}

//////////////////////////////////////////////////////////////////////////////

ElementType& Entities::element_type() const
{
  cf3_assert_desc("element_type not initialized", is_not_null(m_element_type));
  return *m_element_type;
}

////////////////////////////////////////////////////////////////////////////////

boost::shared_ptr< List< Uint > > Entities::create_used_nodes(const Component& node_user, const std::string& space_name)
{
  boost::shared_ptr< List< Uint > > used_nodes = allocate_component< List< Uint > >(mesh::Tags::nodes_used());

  std::vector< Handle<Entities const> > entities_vector = range_to_const_vector(find_components_recursively<Entities>(node_user));
  Handle<Entities const> self_entities(node_user.handle<Component>());
  if(is_not_null(self_entities))
    entities_vector.push_back(self_entities);

  // No entities found, so the list of nodes is empty
  if(entities_vector.empty())
    return used_nodes;

  const Uint all_nb_nodes = entities_vector.front()->space(space_name).fields().size();

  std::vector<bool> node_is_used(all_nb_nodes, false);

  // First count the number of unique nodes
  Uint nb_nodes = 0;
  BOOST_FOREACH(const Handle<Entities const>& entities, entities_vector)
  {
    const Space& space = entities->space(space_name);
    const Uint nb_elems = entities->size();

    for (Uint idx=0; idx<nb_elems; ++idx)
    {
      cf3_assert(idx<space.connectivity().size());
      boost_foreach(const Uint node, space.connectivity()[idx])
      {
        cf3_assert(node<node_is_used.size());
        if(!node_is_used[node])
        {
          node_is_used[node] = true;
          ++nb_nodes;
        }
      }
    }
  }

  // reserve space for all unique nodes
  used_nodes->resize(nb_nodes);
  common::List<Uint>::ListT& nodes_array = used_nodes->array();

  // Add the unique node indices
  node_is_used.assign(all_nb_nodes, false);
  Uint back = 0;
  BOOST_FOREACH(const Handle<Entities const>& entities, entities_vector)
  {
    const Space& space = entities->space(space_name);
    const Uint nb_elems = entities->size();
    for (Uint idx=0; idx<nb_elems; ++idx)
    {
      boost_foreach(const Uint node, space.connectivity()[idx])
      {
        if(!node_is_used[node])
        {
          node_is_used[node] = true;
          nodes_array[back++] = node;
        }
      }
    }
  }

  return used_nodes;
}

common::List<Uint>& Entities::used_nodes(Component& parent, const bool rebuild)
{
  Handle< common::List<Uint> > used_nodes = find_component_ptr_with_tag<common::List<Uint> >(parent,mesh::Tags::nodes_used());
  if (rebuild && is_not_null(used_nodes))
  {
    parent.remove_component(*used_nodes);
    used_nodes.reset();
  }

  if (is_null(used_nodes))
  {
    boost::shared_ptr< List<Uint> > used_nodes_shr = Entities::create_used_nodes(parent,mesh::Tags::geometry());
    used_nodes = Handle< List<Uint> >(used_nodes_shr);
    parent.add_component(used_nodes_shr);
    used_nodes->add_tag(mesh::Tags::nodes_used());
    used_nodes->properties()["brief"] = std::string("The local node indices used by the parent component");
  }

  return *used_nodes;
}

////////////////////////////////////////////////////////////////////////////////

Uint Entities::size() const
{
  return geometry_space().size();
}

////////////////////////////////////////////////////////////////////////////////

Uint Entities::glb_size() const
{
  if (PE::Comm::instance().is_active())
  {
    Uint glb_nb_elems(0);
    const Uint loc_nb_elems(size() );
    PE::Comm::instance().all_reduce(PE::plus(), &loc_nb_elems, 1, &glb_nb_elems);
    return glb_nb_elems;
  }
  else
  {
    return size();
  }
}

common::Table<Uint>::ConstRow Entities::get_nodes(const Uint elem_idx) const
{
  throw ShouldNotBeHere( FromHere(), " This virtual function has to be overloaded. ");
}

////////////////////////////////////////////////////////////////////////////////

Space& Entities::create_space(const std::string& shape_function_builder_name, Dictionary& space_fields)
{
  /// @note Everything for a space is set-up, except the filling of the connectivity table (size=0xnb_states)
  Handle<Space> space = m_spaces_group->create_component<Space>(space_fields.name());
  space->options().configure_option("shape_function",shape_function_builder_name);
  space->set_support(*this);
  space->get_child("fields")->handle<Link>()->link_to(space_fields);
  space->connectivity().create_lookup().add(space_fields);
  space->connectivity().set_row_size(space->nb_states());
  space_fields.add_space( space );
  return *space;
}

////////////////////////////////////////////////////////////////////////////////

Space& Entities::space (const std::string& space_name) const
{
  return *Handle<Space>(m_spaces_group->get_child(space_name));
}

////////////////////////////////////////////////////////////////////////////////

bool Entities::exists_space(const std::string& name) const
{
  return is_not_null(m_spaces_group->get_child(name));
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

//void Entities::signature_create_space ( SignalArgs& node)
//{
//  XML::SignalOptions options( node );
//  options.add_option("name" , std::string("new_space") )
//      .description("Name to add to space");

//  options.add_option("shape_function" , std::string("cf3.mesh.LagrangeP0.Line") )
//      .description("Shape Function to add as space");
//}

//////////////////////////////////////////////////////////////////////////////////

//void Entities::signal_create_space ( SignalArgs& node )
//{
//  XML::SignalOptions options( node );

//  std::string name = options.value<std::string>("name");

//  std::string shape_function_builder = options.value<std::string>("shape_function");

//  Space& space = create_space(name, shape_function_builder);
//}

////////////////////////////////////////////////////////////////////////////////

bool Entities::is_ghost(const Uint idx) const
{
  cf3_assert_desc(to_str(idx)+">="+to_str(size()),idx < size());
  cf3_assert(size() == m_rank->size());
  cf3_assert(idx<m_rank->size());
  return (*m_rank)[idx] != PE::Comm::instance().rank();
}

////////////////////////////////////////////////////////////////////////////////


ElementType& Entity::element_type() const { return comp->element_type(); }
Uint Entity::glb_idx() const { return comp->glb_idx()[idx]; }
Uint Entity::rank() const { return comp->rank()[idx]; }
bool Entity::is_ghost() const { return comp->is_ghost(idx); }
RealMatrix Entity::get_coordinates() const { return comp->get_coordinates(idx); }
void Entity::put_coordinates(RealMatrix& coordinates) const { return comp->put_coordinates(coordinates,idx); }
void Entity::allocate_coordinates(RealMatrix& coordinates) const { return comp->allocate_coordinates(coordinates); }
Connectivity::ConstRow Entity::get_nodes() const { return comp->get_nodes(idx); }
std::ostream& operator<<(std::ostream& os, const Entity& entity)
{
  cf3_assert(is_not_null(entity.comp));
  os << entity.comp->uri().string()<<"["<<entity.idx<<"]";
  return os;
}


bool IsElementsVolume::operator()(const Handle< Entities >& component)
{
  return component->element_type().dimension() == component->element_type().dimensionality();
}

bool IsElementsVolume::operator()(const Entities& component)
{
  return component.element_type().dimension() == component.element_type().dimensionality();
}

bool IsElementsSurface::operator()(const Handle< Entities >& component)
{
  return component->element_type().dimension() == component->element_type().dimensionality() + 1;
}

bool IsElementsSurface::operator()(const Entities& component)
{
  return component.element_type().dimension() == component.element_type().dimensionality() + 1;
}


void Entities::resize(const Uint nb_elem)
{
  rank().resize(nb_elem);
  glb_idx().resize(nb_elem);
  boost_foreach(Space& space, find_components_recursively<Space>(*m_spaces_group))
  {
    space.connectivity().resize(nb_elem);
  }
}


////////////////////////////////////////////////////////////////////////////////
} // mesh
} // cf3
