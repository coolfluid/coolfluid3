// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>

#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/EventHandler.hpp"
#include "common/Builder.hpp"
#include "common/Link.hpp"
#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/StringConversion.hpp"
#include "common/Signal.hpp"
#include "common/Tags.hpp"

#include "common/PE/Comm.hpp"
#include "common/PE/debug.hpp"

#include "common/XML/SignalOptions.hpp"

#include "math/VariablesDescriptor.hpp"

#include "mesh/LibMesh.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/ContinuousDictionary.hpp"
#include "mesh/DiscontinuousDictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/MeshElements.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/WriteMesh.hpp"
#include "mesh/MeshMetadata.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Faces.hpp"
#include "mesh/BoundingBox.hpp"
#include "mesh/Space.hpp"

namespace cf3 {
namespace mesh {

using namespace common;
using namespace common::XML;
using namespace common::PE;

common::ComponentBuilder < Mesh, Component, LibMesh > Mesh_Builder;

////////////////////////////////////////////////////////////////////////////////

Mesh::Mesh ( const std::string& name  ) :
  Component ( name ),
  m_dimension(0u),
  m_dimensionality(0u),
  m_block_mesh_changed(false)
{
  mark_basic(); // by default meshes are visible

  properties().add("local_nb_cells",Uint(0));
  properties().add("local_nb_faces",Uint(0));
  properties().add("local_nb_nodes",Uint(0));
  properties().add("global_nb_cells",Uint(0));
  properties().add("global_nb_faces",Uint(0));
  properties().add("global_nb_nodes",Uint(0));
  properties().add("dimensionality",Uint(0));
  properties().add(common::Tags::dimension(),Uint(0));

  m_topology   = create_static_component<Region>(mesh::Tags::topology());
  m_metadata   = create_static_component<MeshMetadata>("metadata");

  m_local_bounding_box  = create_static_component<BoundingBox>("bounding_box_local");
  m_global_bounding_box = create_static_component<BoundingBox>("bounding_box_global");

  regist_signal ( "write_mesh" )
      .description( "Write mesh, guessing automatically the format" )
      .pretty_name("Write Mesh" )
      .connect   ( boost::bind ( &Mesh::signal_write_mesh,    this, _1 ) )
      .signature ( boost::bind ( &Mesh::signature_write_mesh, this, _1 ) );
      
  regist_signal ( "raise_mesh_loaded" )
      .description( "Raise the mesh loaded event" )
      .pretty_name("Raise Mesh Loaded" )
      .connect   ( boost::bind ( &Mesh::signal_raise_mesh_loaded,    this, _1 ) );

  m_geometry_fields = create_static_component<ContinuousDictionary>(mesh::Tags::geometry());
  m_geometry_fields->add_tag(mesh::Tags::geometry());
  Handle< Field > coord_field = m_geometry_fields->create_static_component< Field >(mesh::Tags::coordinates());
  coord_field->add_tag(mesh::Tags::coordinates());
  coord_field->create_descriptor("coord[vector]");
  m_geometry_fields->m_fields.push_back(coord_field);
}

////////////////////////////////////////////////////////////////////////////////

Mesh::~Mesh()
{
}

////////////////////////////////////////////////////////////////////////////////

void Mesh::initialize_nodes(const Uint nb_nodes, const Uint dimension)
{
  cf3_assert(dimension > 0);

  geometry_fields().coordinates().set_dict(geometry_fields());
  geometry_fields().coordinates().descriptor().options().set(common::Tags::dimension(),dimension);
  geometry_fields().resize(nb_nodes);

  cf3_assert(geometry_fields().size() == nb_nodes);
  cf3_assert(geometry_fields().coordinates().row_size() == dimension);
  m_dimension = dimension;
  properties().property(common::Tags::dimension()) = m_dimension;
  properties().property("local_nb_nodes")  = geometry_fields().size();
  update_structures();
}

////////////////////////////////////////////////////////////////////////////////

void Mesh::update_structures()
{
  Uint entities_idx=0;
  Uint dict_idx=0;
  m_elements.clear();
  m_dictionaries.clear();
  std::map<std::string,Handle<Dictionary> > dicts_map; // sorted by string
  boost_foreach ( Entities& elements, find_components_recursively<Entities>(topology()) )
  {
    m_elements.push_back(elements.handle<Entities>());
    boost_foreach(const Handle<Space>& space, elements.spaces())
    {
      dicts_map[space->dict().uri().string()]=space->dict().handle<Dictionary>();
    }
  }
  m_dictionaries.reserve(dicts_map.size());
  foreach_container( (const std::string& uri) (const Handle<Dictionary>& dict), dicts_map)
  {
    m_dictionaries.push_back(dict);
    dict->update_structures();
  }

  // Set private member m_entities_idx in each Entities
  for (entities_idx=0; entities_idx<m_elements.size(); ++entities_idx)
  {
    m_elements[entities_idx]->m_entities_idx = entities_idx;
  }

  bool found=false;
  boost_foreach (const Handle<Dictionary> dict, m_dictionaries)
  {
    if (dict == m_geometry_fields)
    {
      found = true;
      break;
    }
  }
  if (!found)
  {
    if (m_geometry_fields)
      m_dictionaries.push_back(m_geometry_fields);
  }

  // Set private member m_dict_idx in each Space
  for (dict_idx=0; dict_idx<m_dictionaries.size(); ++dict_idx)
  {
    boost_foreach(const Handle<Space>& space, m_dictionaries[dict_idx]->spaces())
    {
      space->m_dict_idx = dict_idx;
    }
  }

}

////////////////////////////////////////////////////////////////////////////////

void Mesh::update_statistics()
{
  cf3_assert(m_dimension == geometry_fields().coordinates().row_size() );
  boost_foreach ( const Entities& elements, find_components_recursively<Entities>(topology()) )
  {
    m_dimensionality = std::max(m_dimensionality,elements.element_type().dimensionality());
  }

  Uint nb_cells = 0;
  boost_foreach ( Cells& elements, find_components_recursively<Cells>(topology()) )
    nb_cells += elements.size();

  Uint nb_faces = 0;
  boost_foreach ( Faces& elements, find_components_recursively<Faces>(topology()) )
    nb_faces += elements.size();

  properties().property(common::Tags::dimension()) = m_dimension;
  properties().property("dimensionality")= m_dimensionality;
  properties().property("local_nb_cells") = nb_cells;
  properties().property("local_nb_faces") = nb_faces;
  properties().property("local_nb_nodes") = geometry_fields().size();

  std::vector<Uint> global_stats(3);
  global_stats[0]=nb_cells;
  global_stats[1]=nb_faces;
  global_stats[2]=geometry_fields().size();
  if (PE::Comm::instance().is_active())
    PE::Comm::instance().all_reduce(PE::plus(),global_stats,global_stats);
  properties().property("global_nb_cells") = global_stats[0];
  properties().property("global_nb_faces") = global_stats[1];
  properties().property("global_nb_nodes") = global_stats[2];

  m_local_bounding_box->build(*this);
  m_local_bounding_box->update_properties();

  m_global_bounding_box->define(*m_local_bounding_box);
  m_global_bounding_box->make_global();
  m_global_bounding_box->update_properties();
}

////////////////////////////////////////////////////////////////////////////////

Dictionary& Mesh::create_continuous_space( const std::string& space_name, const std::string& space_lib_name)
{
  std::vector< Handle<Region> > regions(1,topology().handle<Region>());
  return create_continuous_space(space_name,space_lib_name,regions);
}

Dictionary& Mesh::create_continuous_space( const std::string& space_name, const std::string& space_lib_name, const std::vector< Handle<Region> >& regions )
{
  std::map< std::string, Handle<Entities> > entities_set; // sorted by uri
  boost_foreach(const Handle<Region>& region, regions)
  {
    boost_foreach(Entities& entities, find_components_recursively<Entities>(*region) )
    {
      entities_set[entities.uri().string()]=entities.handle<Entities>();
    }
  }
  std::vector< Handle<Entities> > entities_vec;
  foreach_container( (const std::string& uri)(const Handle<Entities>& entities), entities_set )
  {
    entities_vec.push_back(entities);
  }
  return create_continuous_space(space_name, space_lib_name, entities_vec);
}

Dictionary& Mesh::create_continuous_space( const std::string& space_name, const std::string& space_lib_name, const std::vector< Handle<Entities> >& entities )
{
  Dictionary& dict = *create_component<ContinuousDictionary>(space_name);

  CFinfo << "Creating Continuous space " << dict.uri() << " ("<<space_lib_name<<") for entities" << CFendl;
  boost_foreach(const Handle<Entities>& entities_handle, entities )
  {
    CFinfo << "    -  " <<  entities_handle->uri() << CFendl;
  }

  boost_foreach(const Handle<Entities>& entities_handle, entities )
  {
    entities_handle->create_space(space_lib_name+"."+entities_handle->element_type().shape_name(),dict);
  }
  dict.build();
  update_structures();
  return dict;
}

////////////////////////////////////////////////////////////////////////////////

Dictionary& Mesh::create_discontinuous_space( const std::string& space_name, const std::string& space_lib_name)
{
  std::vector< Handle<Region> > regions(1,topology().handle<Region>());
  return create_discontinuous_space(space_name,space_lib_name,regions);
}

Dictionary& Mesh::create_discontinuous_space( const std::string& space_name, const std::string& space_lib_name, const std::vector< Handle<Region> >& regions )
{
  std::map< std::string, Handle<Entities> > entities_set; // sorted by uri
  boost_foreach(const Handle<Region>& region, regions)
  {
    boost_foreach(Entities& entities, find_components_recursively<Entities>(*region) )
    {
      entities_set[entities.uri().string()]=entities.handle<Entities>();
    }
  }
  std::vector< Handle<Entities> > entities_vec;
  foreach_container( (const std::string& uri)(const Handle<Entities>& entities), entities_set )
  {
    entities_vec.push_back(entities);
  }
  return create_discontinuous_space(space_name, space_lib_name, entities_vec);
}

Dictionary& Mesh::create_discontinuous_space( const std::string& space_name, const std::string& space_lib_name, const std::vector< Handle<Entities> >& entities )
{
  Dictionary& dict = *create_component<DiscontinuousDictionary>(space_name);

  CFinfo << "Creating Disontinuous space " << dict.uri() << " ("<<space_lib_name<<") for entities" << CFendl;
  boost_foreach(const Handle<Entities>& entities_handle, entities )
  {
    CFinfo << "    -  " <<  entities_handle->uri() << CFendl;
  }

  boost_foreach(const Handle<Entities>& entities_handle, entities )
  {
    entities_handle->create_space(space_lib_name+"."+entities_handle->element_type().shape_name(),dict);
  }
  dict.build();
  update_structures();
  return dict;
}

////////////////////////////////////////////////////////////////////////////////

Dictionary& Mesh::geometry_fields() const
{
  return *m_geometry_fields;
}

//////////////////////////////////////////////////////////////////////////////

void Mesh::signature_write_mesh ( SignalArgs& node)
{
  SignalOptions options( node );

  options.add("file" , URI(name() + ".msh") )
      .description("File to write" ).mark_basic();

  std::vector<URI> fields;
  options.add("fields" , fields )
      .description("Field paths to write");
}

////////////////////////////////////////////////////////////////////////////////

void Mesh::signal_write_mesh ( SignalArgs& node )
{
  SignalOptions options( node );

  URI file(name()+".msh");

  if (options.check("file"))
    file = options.value<URI>("file");

  // check protocol for file loading
  // if( file.scheme() != URI::Scheme::FILE )
  //   throw ProtocolError( FromHere(), "Wrong protocol to access the file, expecting a \'file\' but got \'" + file.string() + "\'" );

  URI fpath( file );
//  if( fpath.scheme() != URI::Scheme::FILE )
//    throw ProtocolError( FromHere(), "Wrong protocol to access the file, expecting a \'file\' but got \'" + fpath.string() + "\'" );

  std::vector<URI> fields;

  if (options.check("fields"))
    fields = options.array<URI>("fields");

  write_mesh(fpath,fields);
}

void Mesh::signal_raise_mesh_loaded ( SignalArgs& node )
{
  raise_mesh_loaded();
}


void Mesh::write_mesh( const URI& file, const std::vector<URI> fields)
{
  Handle<WriteMesh> mesh_writer = create_component<WriteMesh>("writer");
  mesh_writer->write_mesh(*this,file,fields);
  remove_component(*mesh_writer);
}

////////////////////////////////////////////////////////////////////////////////

bool Mesh::check_sanity(std::vector<std::string>& messages) const
{
  Uint nb_messages_init = messages.size();
  if (dimension() == 0)
    messages.push_back("mesh.dimension not configured");

  if (dimensionality() == 0)
    messages.push_back("mesh.dimensionality not configured");

  if (dimensionality() > dimension())
    messages.push_back("dimensionality ["+to_str(dimensionality())+"]  >  dimension ["+to_str(dimension())+"]");

  if(geometry_fields().coordinates().row_size() != dimension())
    messages.push_back("coordinates dimension does not match mesh.dimension");

  boost_foreach(const Dictionary& dict, find_components_recursively<Dictionary>(*this))
  {
    dict.check_sanity(messages);
  }

  if (Comm::instance().size()>1)
  {
    std::set<Uint> unique_node_gids;
    boost_foreach(const Uint gid, geometry_fields().glb_idx().array())
    {
      std::pair<std::set<Uint>::iterator, bool > inserted = unique_node_gids.insert(gid);
      if (inserted.second == false)
      {
        messages.push_back(geometry_fields().glb_idx().uri().string()+" has non-unique entries.  (entry "+to_str(gid)+" exists more than once, no further checks)");
        break;
      }
    }
  }

  std::set<Uint> unique_elem_gids;
  boost_foreach(const Entities& entities, find_components_recursively<Entities>(*this))
  {
    if (entities.rank().size() != entities.size())
      messages.push_back(entities.uri().string()+": size() ["+to_str(entities.size())+"] != rank().size() ["+to_str(entities.rank().size())+"]");
    if (entities.glb_idx().size() != entities.size())
      messages.push_back(entities.uri().string()+": size() ["+to_str(entities.size())+"] != glb_idx().size() ["+to_str(entities.glb_idx().size())+"]");

    if (Comm::instance().size()>1)
    {
      boost_foreach(const Uint gid, entities.glb_idx().array())
      {
        std::pair<std::set<Uint>::iterator, bool > inserted = unique_elem_gids.insert(gid);
        if (inserted.second == false)
        {
          messages.push_back(entities.glb_idx().uri().string()+" has non-unique entries.  (entry "+to_str(gid)+" exists more than once, no further checks)");
          break;
        }
      }
    }
  }

  return (messages.size()-nb_messages_init) == 0;

}

bool Mesh::check_sanity() const
{
  std::vector<std::string> messages;
  bool sane = check_sanity(messages);
  if ( sane == false )
  {
    std::stringstream message;
    message << "Mesh "+uri().string()+" is not sane:"<<std::endl;
    boost_foreach(const std::string& str, messages)
    {
      message << "- " << str << std::endl;
    }
    throw InvalidStructure(FromHere(), message.str() );
  }
  return sane;
}

////////////////////////////////////////////////////////////////////////////////

void Mesh::raise_mesh_loaded()
{
  update_structures();
  update_statistics();

  for (Uint dict_idx=0; dict_idx<m_dictionaries.size(); ++dict_idx)
  {
    m_dictionaries[dict_idx]->rebuild_map_glb_to_loc();
    m_dictionaries[dict_idx]->rebuild_node_to_element_connectivity();
  }

  check_sanity();

  // Raise an event to indicate that this mesh was loaded
  SignalOptions options;
  options.add("mesh_uri", uri());

  SignalArgs f= options.create_frame();
  Core::instance().event_handler().raise_event( Tags::event_mesh_loaded(), f );

  update_statistics();
  check_sanity();
}


////////////////////////////////////////////////////////////////////////////////

void Mesh::raise_mesh_changed()
{
  update_structures();
  update_statistics();

  for (Uint dict_idx=0; dict_idx<m_dictionaries.size(); ++dict_idx)
  {
    m_dictionaries[dict_idx]->rebuild_map_glb_to_loc();
    m_dictionaries[dict_idx]->rebuild_node_to_element_connectivity();
  }

  check_sanity();

  // Raise an event to indicate that this mesh was changed
  SignalOptions options;
  options.add("mesh_uri", uri());

  if(!m_block_mesh_changed)
  {
    SignalArgs f= options.create_frame();
    Core::instance().event_handler().raise_event( Tags::event_mesh_changed(), f );
  }

  update_statistics();
  check_sanity();
}

////////////////////////////////////////////////////////////////////////////////

void Mesh::block_mesh_changed ( const bool block )
{
  m_block_mesh_changed = block;
}


////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
