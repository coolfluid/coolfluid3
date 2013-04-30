// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
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
#include "math/VariablesDescriptor.hpp"

namespace cf3 {
namespace mesh {

using namespace common;
using namespace common::PE;
using namespace common::XML;

RegistTypeInfo<Dictionary, LibMesh> regist_Dictionary_type;

////////////////////////////////////////////////////////////////////////////////

Dictionary::Dictionary ( const std::string& name  ) :
  Component( name ),
  m_dim(0),
  m_is_continuous(true) // default continuous
{
  mark_basic();

  // Static components

  m_rank = create_static_component< common::List<Uint> >("rank");
  m_rank->add_tag("rank");

  m_glb_idx = create_static_component< common::List<Uint> >(mesh::Tags::global_indices());
  m_glb_idx->add_tag(mesh::Tags::global_indices());

  m_glb_to_loc = create_static_component< common::Map<boost::uint64_t,Uint> >(mesh::Tags::map_global_to_local());
  m_glb_to_loc->add_tag(mesh::Tags::map_global_to_local());

  m_connectivity = create_static_component< common::DynTable<SpaceElem> >("element_connectivity");

  options().add("dimension",m_dim).link_to(&m_dim);

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

Uint Dictionary::size() const
{
  return m_glb_idx->size();
}

////////////////////////////////////////////////////////////////////////////////

void Dictionary::resize(const Uint size)
{
  if (m_glb_idx->size() == 0)
  {
    m_glb_idx->resize(size);
    m_rank->resize(size);
    // assume serial
    for (Uint n=0; n<size; ++n)
    {
      m_glb_idx->array()[n]=n;
      m_rank->array()[n]=PE::Comm::instance().rank();
    }
  }
  else
  {
    m_glb_idx->resize(size);
    m_rank->resize(size);
  }
  properties()["size"]=size;
  boost_foreach(Field& field, find_components<Field>(*this))
      field.resize(size);

}

//////////////////////////////////////////////////////////////////////////////

CommPattern& Dictionary::comm_pattern()
{
  if(is_null(m_comm_pattern))
  {
    PE::CommPattern& comm_pattern = *create_component<PE::CommPattern>("CommPattern");
    comm_pattern.insert("gid",glb_idx().array(),false);
    comm_pattern.setup(Handle<PE::CommWrapper>(comm_pattern.get_child("gid")),rank().array());
    m_comm_pattern = Handle<common::PE::CommPattern>(comm_pattern.handle());
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
  const Handle< Space const>& s = m_raw_spaces_map.find(&entities)->second;
  cf3_assert(s);
  return *s;
}

////////////////////////////////////////////////////////////////////////////////

const Handle< Space const>& Dictionary::space(const Handle<Entities const>& entities) const
{
  const Handle< Space const>& s = m_spaces_map.find(entities)->second;
  cf3_assert(s);
  return s;
}

////////////////////////////////////////////////////////////////////////////////

Field& Dictionary::create_field(const std::string &name, const Uint cols)
{
  Handle<Field> field = create_component<Field>(name);
  field->set_dict(*this);
  field->set_var_type(ARRAY);

  // @todo remove next line when ready
  field->create_descriptor(name+"["+to_str(cols)+"]",m_dim);

  field->set_row_size(cols);
  field->resize(size());

  update_structures();

  return *field;
}

////////////////////////////////////////////////////////////////////////////////

Field& Dictionary::create_field(const std::string &name, const VarType var_type)
{
  if (var_type == ARRAY)
    throw SetupError(FromHere(), "Should not call this for array types. use create_field(name,cols)");

  Handle<Field> field = create_component<Field>(name);
  field->set_dict(*this);
  field->set_var_type(var_type);

  // @todo remove next lines when ready
  if (var_type == SCALAR)
    field->create_descriptor(name+"[scalar]",m_dim);
  else if (var_type == VECTOR_2D || var_type == VECTOR_3D)
    field->create_descriptor(name+"[vector]",m_dim);
  else if (var_type == TENSOR_2D || var_type == TENSOR_3D)
    field->create_descriptor(name+"[tensor]",m_dim);

  field->set_row_size( (Uint) var_type );
  field->resize(size());

  update_structures();

  return *field;
}

////////////////////////////////////////////////////////////////////////////////

Field& Dictionary::create_field(const std::string &name, const std::string& description)
{
  Handle<Field> field = create_component<Field>(name);
  field->set_dict(*this);

  // @todo remove next line when ready
  field->create_descriptor(description,m_dim);

  field->set_row_size(field->descriptor().size());
  field->resize(size());

  update_structures();

  return *field;
}


////////////////////////////////////////////////////////////////////////////////

Field& Dictionary::create_field(const std::string &name, math::VariablesDescriptor& variables_descriptor)
{
  CFinfo << "Creating field " << uri()/name << CFendl;
  if (m_dim == 0) throw SetupError(FromHere(), "dimension not configured");
  Handle<Field> field = create_component<Field>(name);
  field->set_dict(*this);
  field->set_descriptor(variables_descriptor);
  if (variables_descriptor.options().option(common::Tags::dimension()).value<Uint>() == 0)
  {
    field->descriptor().options().set(common::Tags::dimension(),m_dim);
  }
  field->set_row_size(field->descriptor().size());
  field->resize(size());

  update_structures();

  CFinfo << "Created field " << field->uri() << " with variables \n";
  for (Uint var=0; var<field->descriptor().nb_vars(); ++var)
  {
    CFinfo << "    - " << field->descriptor().user_variable_name(var) << " [" << field->descriptor().var_length(var) << "]\n";
  }
  CFinfo << CFflush;


  return *field;
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

const std::vector< Handle< Space > >& Dictionary::spaces() const
{
  return m_spaces;
}

////////////////////////////////////////////////////////////////////////////////

Field& Dictionary::field(const std::string& name)
{
  cf3_assert_desc("field "+name+" not found in "+uri().string(),get_child(name));
  return *Handle<Field>(get_child(name));
}

////////////////////////////////////////////////////////////////////////////////

void Dictionary::build()
{
  update_structures();
  rebuild_spaces_from_geometry();
  rebuild_map_glb_to_loc();
  rebuild_node_to_element_connectivity();
  check_sanity();
}

////////////////////////////////////////////////////////////////////////////////

void Dictionary::update_structures()
{
  m_entities.clear();
  m_spaces.clear();
  m_entities.reserve(m_spaces_map.size());
  m_spaces.reserve(m_spaces_map.size());
  std::vector< Handle<Entities const> > to_remove_from_spaces_map;
  std::map< std::string, Handle<Entities const> > tmp_entities_map;
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
      tmp_entities_map[entities->uri().string()] = entities;
//      m_entities.push_back(const_cast<Entities&>(*entities).handle<Entities>());
//      m_spaces.push_back(const_cast<Space&>(*space).handle<Space>());
      m_dim = std::max(m_dim,entities->element_type().dimension());
    }
  }
  options().set("dimension",m_dim);
  foreach_container ( (const std::string& uri)(const Handle<Entities const>& entities), tmp_entities_map)
  {
    m_entities.push_back(const_cast<Entities&>(*entities).handle<Entities>());
    m_spaces.push_back(const_cast<Space&>(*m_spaces_map[entities]).handle<Space>());
  }

  boost_foreach( const Handle<Entities const>& entities, to_remove_from_spaces_map)
  {
    m_spaces_map.erase(entities);
  }

  m_raw_spaces_map.clear();
  foreach_container( (const Handle<Entities const>& entities) (const Handle<Space const>& space), m_spaces_map)
  {
    m_raw_spaces_map[entities.get()]=space;
  }


  m_fields.clear();
  boost_foreach (Field& field, find_components<Field>(*this))
  {
    m_fields.push_back(field.handle<Field>());
  }
}

////////////////////////////////////////////////////////////////////////////////

void Dictionary::rebuild_map_glb_to_loc()
{
  m_glb_to_loc->clear();
  m_glb_to_loc->reserve(size());
  for (Uint n=0; n<size(); ++n)
    m_glb_to_loc->push_back(glb_idx()[n],n);
  m_glb_to_loc->sort_keys();
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
    if (Handle< Component > found = get_child(mesh::Tags::coordinates()))
    {
      m_coordinates = found->handle<Field>();
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
    m_coordinates.reset();

  Field& coordinates = create_field("coordinates","coords[vector]");
  CFdebug << "    - Interpolating coordinates from geometry" << CFendl;
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
      coords.noalias() = interpolation*geom_nodes;

      for (Uint i=0; i<sf.nb_nodes(); ++i)
      {
        const Uint pt = entities_space.connectivity()[e][i];
        for(Uint d=0; d<coords.cols(); ++d)
          coordinates[pt][d] = coords(i,d);
      }
    }
  }

  m_coordinates = coordinates.handle<Field>();
  m_coordinates->add_tag(mesh::Tags::coordinates());

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

  options.add("name",std::string(""))
      .description("Name of the field" );

  options.add("variables",std::string(""))
      .description("Variables description of the field" );

  options.add("size",Uint(1u))
      .description("Variable length" );

}

////////////////////////////////////////////////////////////////////////////////

void Dictionary::signal_create_field( SignalArgs& node )
{
  SignalOptions options( node );

  std::string name = options.value<std::string>("name");
  std::string variables = options.value<std::string>("variables");
  Uint cols = options.value<Uint>("size");

  Handle<Field> field;
  if ( variables.empty() && !name.empty() )
    field = create_field(name,cols).handle<Field>();
  else if ( !variables.empty() && !name.empty() )
    field = create_field(name,variables).handle<Field>();
  else
    throw SetupError(FromHere(), "name and variables/size must be specified");

  SignalFrame reply = node.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", field->uri());
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
