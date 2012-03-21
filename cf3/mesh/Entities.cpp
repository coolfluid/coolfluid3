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
#include "mesh/Functions.hpp"

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

}

////////////////////////////////////////////////////////////////////////////////

Entities::~Entities()
{
}

//////////////////////////////////////////////////////////////////////////////

void Entities::initialize(const std::string& element_type_name, Dictionary& geometry_dict)
{
  options().configure_option("element_type",element_type_name);
  cf3_assert(is_not_null(m_element_type));
  create_geometry_space(geometry_dict);
}

void Entities::create_geometry_space(Dictionary& geometry_dict)
{
  if ( is_null(m_element_type) )
    throw SetupError(FromHere(),"option 'element_type' needs to be configured first");

  m_geometry_dict = geometry_dict.handle<Dictionary>();
  if ( is_not_null(m_spaces_group->get_child(mesh::Tags::geometry())) )
  {
    space(geometry_dict).options().configure_option("shape_function",m_element_type->shape_function().derived_type_name());
  }
  else
  {
    m_geometry_space = m_spaces_group->create_component<Space>(geometry_dict.name());
    m_spaces_vector.push_back(m_geometry_space);
    m_geometry_space->initialize(*this,geometry_dict);
    m_geometry_space->options().configure_option("shape_function",m_element_type->shape_function().derived_type_name());
    m_geometry_space->add_tag(mesh::Tags::geometry());
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
    const Dictionary& dict = find_components_recursively<Entities>(parent).begin()->geometry_fields();
    boost::shared_ptr< List<Uint> > used_nodes_shr = build_used_nodes_list(parent, dict, true);
    used_nodes = Handle< List<Uint> >(used_nodes_shr);
    parent.add_component(used_nodes_shr);
    used_nodes->add_tag(mesh::Tags::nodes_used());
    used_nodes->properties()["brief"] = std::string("The local node indices used by the parent component");
  }

  return *used_nodes;
}

////////////////////////////////////////////////////////////////////////////////

Space& Entities::space (const Dictionary& dict)
{
  return const_cast<Space&>(dict.space(*this));
}

////////////////////////////////////////////////////////////////////////////////

const Space& Entities::space (const Dictionary& dict) const
{
  return dict.space(*this);
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

////////////////////////////////////////////////////////////////////////////////

Space& Entities::create_space(const std::string& shape_function_builder_name, Dictionary& space_dict)
{
  /// @note Everything for a space is set-up, except the filling of the connectivity table (size=0xnb_states)
  Handle<Space> space = m_spaces_group->create_component<Space>(space_dict.name());
  m_spaces_vector.push_back(space);
  space->initialize(*this,space_dict);
  space->options().configure_option("shape_function",shape_function_builder_name);
  return *space;
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


ElementType& Entity::element_type() const { return comp->element_type(); }
Uint Entity::glb_idx() const { return comp->glb_idx()[idx]; }
Uint Entity::rank() const { return comp->rank()[idx]; }
bool Entity::is_ghost() const { return comp->is_ghost(idx); }
RealMatrix Entity::get_coordinates() const { return comp->geometry_space().get_coordinates(idx); }
void Entity::put_coordinates(RealMatrix& coordinates) const { return comp->geometry_space().put_coordinates(coordinates,idx); }
void Entity::allocate_coordinates(RealMatrix& coordinates) const { return comp->geometry_space().allocate_coordinates(coordinates); }
Connectivity::ConstRow Entity::get_nodes() const { return comp->geometry_space().connectivity()[idx]; }
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
