// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "common/Log.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"
#include "common/Builder.hpp"
#include "common/Link.hpp"
#include "common/FindComponents.hpp"
#include "common/Core.hpp"
#include "common/EventHandler.hpp"
#include "common/StringConversion.hpp"
#include "common/Tags.hpp"
#include "common/DynTable.hpp"
#include "common/List.hpp"

#include "common/XML/SignalOptions.hpp"

#include "common/PE/Comm.hpp"
#include "common/PE/CommPattern.hpp"

#include "math/VariablesDescriptor.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Region.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Space.hpp"
#include "mesh/Entities.hpp"
#include "mesh/Connectivity.hpp"

#include "math/Consts.hpp"

namespace cf3 {
namespace mesh {

using namespace common;
using namespace common::PE;
using namespace common::XML;

////////////////////////////////////////////////////////////////////////////////

Dictionary::Dictionary ( const std::string& name  ) :
  Component( name ),
  m_size(0u),
  m_is_continuous(true) // default continuous
{
  mark_basic();

  // Static components

  m_rank = create_static_component< common::List<Uint> >("rank");
  m_rank->add_tag("rank");

  m_glb_idx = create_static_component< common::List<Uint> >(mesh::Tags::global_indices());
  m_glb_idx->add_tag(mesh::Tags::global_indices());

  // Event handlers
  //  Core::instance().event_handler().connect_to_event("mesh_loaded", this, &Dictionary::on_mesh_changed_event);
  Core::instance().event_handler().connect_to_event("mesh_changed", this, &Dictionary::on_mesh_changed_event);

  // Signals
  regist_signal ( "create_field" )
      .description( "Create Field" )
      .pretty_name("Create Field" )
      .connect   ( boost::bind ( &Dictionary::signal_create_field,    this, _1 ) )
      .signature ( boost::bind ( &Dictionary::signature_create_field, this, _1 ) );
}

////////////////////////////////////////////////////////////////////////////////

void Dictionary::add_space(const Handle<Space>& space)
{
  m_spaces_map.insert( std::make_pair(space->support().handle<Entities>(),space ) );
}

////////////////////////////////////////////////////////////////////////////////

Dictionary::~Dictionary()
{
}

////////////////////////////////////////////////////////////////////////////////

void Dictionary::resize(const Uint size)
{
  m_size = size;
  m_glb_idx->resize(m_size);
  m_rank->resize(m_size);
  properties()["size"]=m_size;

  boost_foreach(Field& field, find_components<Field>(*this))
    field.resize(m_size);
}

//////////////////////////////////////////////////////////////////////////////

CommPattern& Dictionary::comm_pattern()
{
  if(is_null(m_comm_pattern))
  {
    PE::CommPattern& comm_pattern = *create_component<PE::CommPattern>("CommPattern");
    comm_pattern.insert("gid",glb_idx().array(),false);
    comm_pattern.setup(Handle<PE::CommWrapper>(comm_pattern.get_child("gid")),rank().array());
    m_comm_pattern = Handle<common::PE::CommPattern>(comm_pattern.handle<Component>());
  }

  return *m_comm_pattern;
}

//////////////////////////////////////////////////////////////////////////////

bool Dictionary::is_ghost(const Uint idx) const
{
  cf3_assert_desc(to_str(idx)+">="+to_str(size()),idx < size());
  cf3_assert(size() == m_rank->size());
  cf3_assert(idx<m_rank->size());
  return (*m_rank)[idx] != Comm::instance().rank();
}

////////////////////////////////////////////////////////////////////////////////

const Space& Dictionary::space(const Entities& entities) const
{
  return *space(entities.handle<Entities>());
}

////////////////////////////////////////////////////////////////////////////////

const Handle< Space const>& Dictionary::space(const Handle<Entities const>& entities) const
{
  const Handle< Space const>& s = m_spaces_map.find(entities)->second;
  cf3_assert(s);
  return s;
}

////////////////////////////////////////////////////////////////////////////////

Field& Dictionary::create_field(const std::string &name, const std::string& variables_description)
{

  Field& field = *create_component<Field>(name);
  field.set_dict(*this);

  if (variables_description == "scalar_same_name")
    field.create_descriptor(name+"[scalar]",Handle<Mesh>(parent())->dimension());
  else
    field.create_descriptor(variables_description,Handle<Mesh>(parent())->dimension());

  field.resize(m_size);
  return field;
}

////////////////////////////////////////////////////////////////////////////////

Field& Dictionary::create_field(const std::string &name, math::VariablesDescriptor& variables_descriptor)
{
  Field& field = *create_component<Field>(name);
  field.set_dict(*this);
  field.set_descriptor(variables_descriptor);
  if (variables_descriptor.options().option(common::Tags::dimension()).value<Uint>() == 0)
    field.descriptor().options().configure_option(common::Tags::dimension(),Handle<Mesh>(parent())->dimension());
  field.resize(m_size);
  return field;
}

////////////////////////////////////////////////////////////////////////////////

bool Dictionary::check_sanity(std::vector<std::string>& messages) const
{
  Uint nb_messages_init = messages.size();
  if (rank().size() != size())
    messages.push_back(uri().string()+": size() ["+to_str(size())+"] != rank().size() ["+to_str(rank().size())+"]");

  if (glb_idx().size() != size())
    messages.push_back(uri().string()+": size() ["+to_str(size())+"] != glb_idx().size() ["+to_str(glb_idx().size())+"]");

  std::set<Uint> unique_gids;
  if (Comm::instance().size()>1)
  {
    for (Uint i=0; i<size(); ++i)
    {
      if (rank()[i] >= PE::Comm::instance().size())
      {
        messages.push_back(rank().uri().string()+"["+to_str(i)+"] has invalid entry. (entry = "+to_str(rank()[i])+" , no further checks)");
        break;
      }
    }
    for (Uint i=0; i<size(); ++i)
    {
      std::pair<std::set<Uint>::iterator, bool > inserted = unique_gids.insert(glb_idx()[i]);
      if (inserted.second == false)
      {
        messages.push_back(glb_idx().uri().string()+"["+to_str(i)+"] has non-unique entries.  (entry "+to_str(glb_idx()[i])+" exists more than once, no further checks)");
        break;
      }
    }
  }


  boost_foreach(const Field& field, find_components_recursively<Field>(*this))
  {
    if (field.size() != size())
      messages.push_back(uri().string()+": size() ["+to_str(size())+"] != "+field.uri().string()+".size() ["+to_str(field.size())+"]");
  }
  // Sane if number of messages did not grow in size
  return messages.size() == nb_messages_init;
}

bool Dictionary::check_sanity() const
{
  std::vector<std::string> messages;
  bool sane = check_sanity(messages);
  if ( sane == false )
  {
    std::stringstream message;
    message << "Dictionary "+uri().string()+" is not sane:"<<std::endl;
    boost_foreach(const std::string& str, messages)
    {
      message << "- " << str << std::endl;
    }
    throw InvalidStructure(FromHere(), message.str() );
  }
  return sane;
}

////////////////////////////////////////////////////////////////////////////////

const std::vector< Handle< Entities > >& Dictionary::entities_range() const
{
  return m_entities;
}

////////////////////////////////////////////////////////////////////////////////

Field& Dictionary::field(const std::string& name)
{
  cf3_assert_desc("field "+name+" not found in "+uri().string(),get_child(name));
  return *Handle<Field>(get_child(name));
}

////////////////////////////////////////////////////////////////////////////////

void Dictionary::on_mesh_changed_event( SignalArgs& args )
{
  common::XML::SignalOptions options( args );


  URI mesh_uri = options.value<URI>("mesh_uri");
  if (mesh_uri.is_relative())
  {
    throw InvalidURI(FromHere(),"URI "+to_str(mesh_uri)+" should be absolute");
  }
  Mesh& mesh_arg  = *Handle<Mesh>(access_component(mesh_uri));
  Mesh& this_mesh = *Handle<Mesh>(parent());

  if (&this_mesh == &mesh_arg)
  {
    if (m_spaces_map.size())
      update();
  }
}

////////////////////////////////////////////////////////////////////////////////

void Dictionary::update()
{
  m_entities.clear();
  m_spaces.clear();
  m_entities.reserve(m_spaces_map.size());
  m_spaces.reserve(m_spaces_map.size());

  std::vector< Handle<Entities const> > to_remove_from_spaces_map;
  to_remove_from_spaces_map.reserve(m_spaces_map.size());
  foreach_container( (const Handle<Entities const>& entities) (const Handle<Space const>& space), m_spaces_map)
  {
    if ( is_null(entities) )
    {
      to_remove_from_spaces_map.push_back(entities);
    }
    else
    {
      cf3_assert(space);
      m_entities.push_back(const_cast<Entities&>(*entities).handle<Entities>());
      m_spaces.push_back(const_cast<Space&>(*space).handle<Space>());
    }
  }
  boost_foreach( const Handle<Entities const>& entities, to_remove_from_spaces_map)
  {
    m_spaces_map.erase(entities);
  }

  if(has_tag(mesh::Tags::geometry()) == false)
    create_connectivity_in_space();
  check_sanity();
}

////////////////////////////////////////////////////////////////////////////////

bool Dictionary::defined_for_entities(const Handle<Entities const>& entities) const
{
  return ( m_spaces_map.find(entities) != m_spaces_map.end() );
}

////////////////////////////////////////////////////////////////////////////////

bool Dictionary::has_coordinates() const
{
  return is_not_null(m_coordinates);
}

////////////////////////////////////////////////////////////////////////////////

Field& Dictionary::coordinates()
{
  if (is_null(m_coordinates))
  {
    if (Handle< Field > found = find_component_ptr_with_tag<Field>(*this,mesh::Tags::coordinates()))
    {
      m_coordinates = found;
    }
    else
    {
      throw ValueNotFound(FromHere(),"Dictionary ["+uri().string()+"] has no coordinates field");
    }
  }
  return *m_coordinates;
}

////////////////////////////////////////////////////////////////////////////////

const Field& Dictionary::coordinates() const
{
  if (is_null(m_coordinates))
    throw ValueNotFound(FromHere(),"Dictionary ["+uri().string()+"] has no coordinates field");
  return *m_coordinates;
}

////////////////////////////////////////////////////////////////////////////////

Field& Dictionary::create_coordinates()
{
  if (has_coordinates())
    throw ValueExists(FromHere(),"coordinates cannot be created, they already exist");

  Field& coordinates = create_field("coordinates","coords[vector]");
  boost_foreach(const Handle<Entities>& entities_handle, entities_range())
  {
    Entities& entities = *entities_handle;
    Space& geometry_space = entities.geometry_space();
    const ShapeFunction& geom_sf = geometry_space.shape_function();
    RealMatrix geom_nodes;
    entities.geometry_space().allocate_coordinates(geom_nodes);

    const Space& entities_space = space(entities);
    const ShapeFunction& sf = entities_space.shape_function();
    const RealMatrix& local_coords = sf.local_coordinates();

    RealMatrix interpolation(sf.nb_nodes(),geom_sf.nb_nodes());
    for (Uint i=0; i<sf.nb_nodes(); ++i)
      interpolation.row(i) = geom_sf.value(local_coords.row(i));

    RealMatrix coords(sf.nb_nodes(),geom_nodes.cols());

    for (Uint e=0; e<entities.size(); ++e)
    {
      entities.geometry_space().put_coordinates(geom_nodes,e);
      coords = interpolation*geom_nodes;

      for (Uint i=0; i<sf.nb_nodes(); ++i)
      {
        const Uint pt = entities_space.connectivity()[e][i];
        for(Uint d=0; d<coords.cols(); ++d)
          coordinates[pt][d] = coords(i,d);
      }
    }
  }

  m_coordinates = Handle<Field>(coordinates.handle<Component>());
  return coordinates;
}

////////////////////////////////////////////////////////////////////////////////

DynTable<Uint>& Dictionary::glb_elem_connectivity()
{
  if (is_null(m_glb_elem_connectivity))
  {
    m_glb_elem_connectivity = create_static_component< DynTable<Uint> >("glb_elem_connectivity");
    m_glb_elem_connectivity->add_tag("glb_elem_connectivity");
    m_glb_elem_connectivity->resize(size());
  }
  return *m_glb_elem_connectivity;
}



////////////////////////////////////////////////////////////////////////////////

void Dictionary::signature_create_field( SignalArgs& node )
{
  SignalOptions options( node );

  options.add_option<std::string>("name")
      .description("Name of the field" );

  options.add_option<std::string>("variables")
      .description("Variables description of the field" );

}

////////////////////////////////////////////////////////////////////////////////

void Dictionary::signal_create_field( SignalArgs& node )
{
  SignalOptions options( node );

  std::string name = options.value<std::string>("name");
  std::string variables = name;
  if(options.check("variables"))
  {
    variables = options.value<std::string>("variables");
  }
  Field& created_component = create_field(name,variables);

  SignalFrame reply = node.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add_option("created_component", created_component.uri());
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
