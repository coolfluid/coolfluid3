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
  m_dimensionality(0u)
{
  mark_basic(); // by default meshes are visible

  properties().add_property("nb_cells",Uint(0));
  properties().add_property("nb_faces",Uint(0));
  properties().add_property("nb_nodes",Uint(0));
  properties().add_property("dimensionality",Uint(0));
  properties().add_property(common::Tags::dimension(),Uint(0));

  m_elements   = create_static_component<MeshElements>("elements");
  m_topology   = create_static_component<Region>(mesh::Tags::topology());
  m_metadata   = create_static_component<MeshMetadata>("metadata");

  regist_signal ( "write_mesh" )
      .description( "Write mesh, guessing automatically the format" )
      .pretty_name("Write Mesh" )
      .connect   ( boost::bind ( &Mesh::signal_write_mesh,    this, _1 ) )
      .signature ( boost::bind ( &Mesh::signature_write_mesh, this, _1 ) );

  m_geometry_fields = create_static_component<ContinuousDictionary>(mesh::Tags::geometry());
  m_geometry_fields->add_tag(mesh::Tags::geometry());
  Handle< Field > coord_field = m_geometry_fields->create_static_component< Field >(mesh::Tags::coordinates());
  coord_field->add_tag(mesh::Tags::coordinates());
  coord_field->create_descriptor("coord[vector]");


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
  geometry_fields().coordinates().descriptor().options().configure_option(common::Tags::dimension(),dimension);
  geometry_fields().resize(nb_nodes);

  cf3_assert(geometry_fields().size() == nb_nodes);
  cf3_assert(geometry_fields().coordinates().row_size() == dimension);
  m_dimension = dimension;
  properties().property(common::Tags::dimension()) = m_dimension;
  properties().property("nb_nodes")  = geometry_fields().size();
}

////////////////////////////////////////////////////////////////////////////////

void Mesh::update_statistics()
{
  cf3_assert(m_dimension == geometry_fields().coordinates().row_size() );
  boost_foreach ( Entities& elements, find_components_recursively<Entities>(topology()) )
    m_dimensionality = std::max(m_dimensionality,elements.element_type().dimensionality());

  Uint nb_cells = 0;
  boost_foreach ( Cells& elements, find_components_recursively<Cells>(topology()) )
    nb_cells += elements.size();

  Uint nb_faces = 0;
  boost_foreach ( Faces& elements, find_components_recursively<Faces>(topology()) )
    nb_faces += elements.size();

  properties().property(common::Tags::dimension()) = m_dimension;
  properties().property("dimensionality")= m_dimensionality;
  properties().property("nb_cells") = nb_cells;
  properties().property("nb_faces") = nb_faces;
  properties().property("nb_nodes") = geometry_fields().size();
}

////////////////////////////////////////////////////////////////////////////////

Dictionary& Mesh::create_continuous_space( const std::string& space_name, const std::string& space_lib_name)
{
  std::vector< Handle<Region> > regions(1,topology().handle<Region>());
  return create_continuous_space(space_name,space_lib_name,regions);
}

Dictionary& Mesh::create_continuous_space( const std::string& space_name, const std::string& space_lib_name, const std::vector< Handle<Region> >& regions )
{
  std::set< Handle<Entities> > entities_set;
  boost_foreach(const Handle<Region>& region, regions)
  {
    boost_foreach(Entities& entities, find_components_recursively<Entities>(*region) )
    {
      entities_set.insert(entities.handle<Entities>());
    }
  }
  std::vector< Handle<Entities> > entities_vec (entities_set.begin(),entities_set.end());
  return create_continuous_space(space_name, space_lib_name, entities_vec);
}

Dictionary& Mesh::create_continuous_space( const std::string& space_name, const std::string& space_lib_name, const std::vector< Handle<Entities> >& entities )
{
  Dictionary& space_fields = *create_component<ContinuousDictionary>(space_name);

  boost_foreach(const Handle<Entities>& entities_handle, entities )
  {
    entities_handle->create_space(space_lib_name+"."+entities_handle->element_type().shape_name(),space_fields);
  }
  space_fields.update();

  CFinfo << "Continuous space " << space_fields.uri() << " ("<<space_lib_name<<") created for entities" << CFendl;
  boost_foreach(const Handle<Entities>& entities_handle, entities )
  {
    CFinfo << "    -  " <<  entities_handle->uri() << CFendl;
  }
  space_fields.update();
  return space_fields;
}

////////////////////////////////////////////////////////////////////////////////

Dictionary& Mesh::create_discontinuous_space( const std::string& space_name, const std::string& space_lib_name)
{
  std::vector< Handle<Region> > regions(1,topology().handle<Region>());
  return create_discontinuous_space(space_name,space_lib_name,regions);
}

Dictionary& Mesh::create_discontinuous_space( const std::string& space_name, const std::string& space_lib_name, const std::vector< Handle<Region> >& regions )
{
  std::set< Handle<Entities> > entities_set;
  boost_foreach(const Handle<Region>& region, regions)
  {
    boost_foreach(Entities& entities, find_components_recursively<Entities>(*region) )
    {
      entities_set.insert(entities.handle<Entities>());
    }
  }
  std::vector< Handle<Entities> > entities_vec (entities_set.begin(),entities_set.end());
  return create_discontinuous_space(space_name, space_lib_name, entities_vec);
}

Dictionary& Mesh::create_discontinuous_space( const std::string& space_name, const std::string& space_lib_name, const std::vector< Handle<Entities> >& entities )
{
  Dictionary& space_fields = *create_component<DiscontinuousDictionary>(space_name);

  boost_foreach(const Handle<Entities>& entities_handle, entities )
  {
    entities_handle->create_space(space_lib_name+"."+entities_handle->element_type().shape_name(),space_fields);
  }
  space_fields.update();

  CFinfo << "Discontinuous space " << space_fields.uri() << " ("<<space_lib_name<<") created for entities" << CFendl;
  boost_foreach(const Handle<Entities>& entities_handle, entities )
  {
    CFinfo << "    -  " <<  entities_handle->uri() << CFendl;
  }
  return space_fields;
}

////////////////////////////////////////////////////////////////////////////////

Dictionary& Mesh::geometry_fields() const
{
  return *m_geometry_fields;
}

////////////////////////////////////////////////////////////////////////////////

MeshElements& Mesh::elements() const
{
  return *m_elements;
}

//////////////////////////////////////////////////////////////////////////////

void Mesh::signature_write_mesh ( SignalArgs& node)
{
  SignalOptions options( node );

  options.add_option("file" , name() + ".msh" )
      .description("File to write" );

//  boost_foreach (Field& field, find_components_recursively<Field>(*this))
//  {
//    options.add_option(field.name(), false )
//        .description("Mark if field gets to be written");
//  }
}

////////////////////////////////////////////////////////////////////////////////

void Mesh::signal_write_mesh ( SignalArgs& node )
{
  SignalOptions options( node );

  std::string file = name()+".msh";

  if (options.check("file"))
    file = options.value<std::string>("file");

  // check protocol for file loading
  // if( file.scheme() != URI::Scheme::FILE )
  //   throw ProtocolError( FromHere(), "Wrong protocol to access the file, expecting a \'file\' but got \'" + file.string() + "\'" );

  URI fpath( file );
//  if( fpath.scheme() != URI::Scheme::FILE )
//    throw ProtocolError( FromHere(), "Wrong protocol to access the file, expecting a \'file\' but got \'" + fpath.string() + "\'" );

  std::vector<URI> fields;

  boost_foreach( Field& field, find_components_recursively<Field>(*this))
  {
    fields.push_back(field.uri());
  }

  write_mesh(fpath,fields);
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
  geometry_fields().update();
  update_statistics();
  elements().update();
  check_sanity();

  // Raise an event to indicate that a mesh was loaded happened
  SignalOptions options;
  options.add_option("mesh_uri", uri());

  SignalArgs f= options.create_frame();
  Core::instance().event_handler().raise_event( "mesh_loaded", f );

  update_statistics();
  elements().update();
  check_sanity();
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
