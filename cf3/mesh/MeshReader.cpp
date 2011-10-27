// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Core.hpp"
#include "common/EventHandler.hpp"
#include "common/Foreach.hpp"
#include "common/BoostFilesystem.hpp"
#include "common/Signal.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionURI.hpp"
#include "common/FindComponents.hpp"


#include "common/PE/Comm.hpp"
#include "common/PE/all_reduce.hpp"
#include "common/PE/operations.hpp"

#include "common/XML/SignalOptions.hpp"

#include "mesh/MeshReader.hpp"
#include "mesh/Region.hpp"
#include "mesh/Domain.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Faces.hpp"
#include "mesh/Elements.hpp"

namespace cf3 {
namespace mesh {

using namespace common;
using namespace common::XML;

////////////////////////////////////////////////////////////////////////////////

MeshReader::MeshReader ( const std::string& name  ) :
  Component ( name )
{
  mark_basic();

  // signals
  regist_signal( "read" )
    ->connect( boost::bind( &MeshReader::signal_read, this, _1 ) )
    ->description("reads a mesh")
    ->pretty_name("Read mesh");
  signal("read")->signature( boost::bind(&MeshReader::read_signature, this, _1) );
}

////////////////////////////////////////////////////////////////////////////////

MeshReader::~MeshReader()
{
}

//////////////////////////////////////////////////////////////////////////////

void MeshReader::signal_read( SignalArgs& node  )
{
  SignalOptions options( node );

  URI path = options.value<URI>("location");

  if( path.scheme() != URI::Scheme::CPATH )
    throw ProtocolError( FromHere(), "Wrong protocol to access the location component, expecting a \'cpath\' but got \'" + path.string() +"\'");

  // get the domain
  Component::Ptr location = access_component_ptr_checked( path );

  std::vector<URI> files = options.array<URI>("files");

  // check protocol for file loading
  boost_foreach(URI file, files)
  {
    if( file.empty() || file.scheme() != URI::Scheme::FILE )
      throw ProtocolError( FromHere(), "Wrong protocol to access the file, expecting a \'file\' but got \'" + file.string() + "\'" );
  }

  // create a mesh in the domain
  if( !files.empty() )
  {
    Mesh& mesh = location->create_component<Mesh>("Mesh");

    // Get the file paths
    boost_foreach(const URI& file, files)
    {
      do_read_mesh_into(file, mesh);
    }
  }
  else
  {
    throw BadValue( FromHere(), "No mesh was read because no files were selected." );
  }
}

//////////////////////////////////////////////////////////////////////////////

void MeshReader::read_mesh_into(const URI& path, Mesh& mesh)
{
  // Call the concrete implementation

  do_read_mesh_into(path, mesh);

  // Raise an event to indicate that a mesh was loaded happened

  SignalOptions options;
  options.add_option< OptionURI >("mesh_uri", mesh.uri());

  SignalFrame f= options.create_frame();
  Core::instance().event_handler().raise_event( "mesh_loaded", f );
}

//////////////////////////////////////////////////////////////////////////////

//Mesh::Ptr MeshReader::create_mesh_from(const URI& file)
//{
//  // Create the mesh
//  Mesh::Ptr mesh ( allocate_component<Mesh>("mesh") );

//  // Call implementation
//  do_read_mesh_into(file,*mesh);

//  // return the mesh
//  return mesh;
//}

//////////////////////////////////////////////////////////////////////////////

std::map<std::string,Elements::Ptr>
  MeshReader::create_cells_in_region (Region& parent_region, SpaceFields& nodes,
                                       const std::vector<std::string>& etypes)
{
  std::map<std::string,Elements::Ptr> cells_map;
  boost_foreach(const std::string& etype, etypes)
  {
    ElementType::Ptr element_type = build_component_abstract_type<ElementType>(etype,etype);
    if (element_type->dimensionality() == element_type->dimension())
    {
      Cells& etype_cells = *parent_region.create_component_ptr<Cells>(element_type->shape_name());
      etype_cells.initialize(etype,nodes);
      cells_map[etype] = etype_cells.as_ptr<Elements>();
    }
  }
  return cells_map;
}

////////////////////////////////////////////////////////////////////////////////

std::map<std::string,Elements::Ptr>
  MeshReader::create_faces_in_region (Region& parent_region, SpaceFields& nodes,
                                       const std::vector<std::string>& etypes)
{
  std::map<std::string,Elements::Ptr> faces_map;
  boost_foreach(const std::string& etype, etypes)
  {
    ElementType::Ptr element_type = build_component_abstract_type<ElementType>(etype,etype);
    if (element_type->dimensionality() == element_type->dimension() - 1)
    {
      Faces& etype_faces = *parent_region.create_component_ptr<Faces>(element_type->shape_name());
      etype_faces.initialize(etype,nodes);
      faces_map[etype] = etype_faces.as_ptr<Elements>();
    }
  }
  return faces_map;
}

////////////////////////////////////////////////////////////////////////////////

std::map<std::string,common::Table<Uint>::Buffer::Ptr>
  MeshReader::create_connectivity_buffermap (std::map<std::string,Elements::Ptr>& elems_map)
{
  // Create regions for each element type
  std::map<std::string,common::Table<Uint>::Buffer::Ptr> buffermap;
  foreach_container((const std::string& etype)(Elements::Ptr elements), elems_map)
  {
    buffermap[etype] = elements->node_connectivity().create_buffer_ptr();
  }
  return buffermap;
}

////////////////////////////////////////////////////////////////////////////////

void MeshReader::remove_empty_element_regions(Region& parent_region)
{
  boost_foreach(Elements& region, find_components_recursively<Elements>(parent_region))
  {
    // find the empty regions
    Uint empty_on_this_rank = region.node_connectivity().array().empty();
    Uint empty_on_all_ranks = empty_on_this_rank;

    /// @todo boolean type had to be converted to Uint for it to work
    if (PE::Comm::instance().is_active())
      PE::Comm::instance().instance().all_reduce( PE::logical_and(), &empty_on_this_rank, 1, &empty_on_all_ranks);

    if ( empty_on_all_ranks )
    {
      Elements::Ptr removed = boost::dynamic_pointer_cast<Elements>(region.parent().remove_component(region.name()));
      removed.reset();
    }
  }

  // loop over regions
  boost_foreach(Region& region, find_components_recursively<Region>(parent_region))
  {
    // find the empty regions
    if ( find_components<Region>(region).empty() && find_components<Elements>(region).empty() )
      {
        Region::Ptr removed = boost::dynamic_pointer_cast<Region>(region.parent().remove_component(region.name()));
        removed.reset();
      }
  }
}

////////////////////////////////////////////////////////////////////////////////

void MeshReader::read_signature( SignalArgs& node )
{
  SignalOptions options( node );

  std::vector<URI> dummy;

  options.add_option< OptionURI >("location", URI() )
      ->description("Component to load mesh into")
      ->cast_to<OptionURI>()->supported_protocol( URI::Scheme::CPATH );

  options.add_option< OptionURI >("files", URI("", URI::Scheme::FILE) )
      ->description("Files to read")
      ->cast_to<OptionURI>()->supported_protocol( URI::Scheme::FILE );
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
