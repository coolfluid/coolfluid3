// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "common/Signal.hpp"
#include "common/FindComponents.hpp"
#include "common/Builder.hpp"
#include "common/Core.hpp"
#include "common/Foreach.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"
#include "common/PropertyList.hpp"

#include "common/LibLoader.hpp"

#include "common/XML/SignalOptions.hpp"

#include "mesh/MeshReader.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Domain.hpp"

#include "mesh/LoadMesh.hpp"


namespace cf3 {
namespace mesh {

using namespace common;
using namespace common::XML;
using namespace cf3::mesh;

common::ComponentBuilder < LoadMesh, Component, LibMesh > LoadMesh_Builder;

////////////////////////////////////////////////////////////////////////////////

LoadMesh::LoadMesh ( const std::string& name  ) :
  Component ( name )
{
  // properties

  properties()["brief"] = std::string("Loads meshes, guessing automatically the format from the file extension");
  mark_basic();

  // signals

  regist_signal ( "load_mesh" )
      .description( "Loads meshes, guessing automatically the format" )
      .pretty_name("Load Mesh" )
      .connect ( boost::bind ( &LoadMesh::signal_load_mesh, this, _1 ) )
      .signature(boost::bind(&LoadMesh::signature_load_mesh, this, _1));

  signal("create_component")->hidden(true);
  signal("rename_component")->hidden(true);
  signal("delete_component")->hidden(true);
  signal("move_component")->hidden(true);
}

LoadMesh::~LoadMesh() {}

////////////////////////////////////////////////////////////////////////////////

void LoadMesh::update_list_of_available_readers()
{
  m_extensions_to_readers.clear();
  
  // TODO proper way to find the list of potential readers
  const std::vector<std::string> known_readers = boost::assign::list_of
    ("cf3.mesh.CGNS.Reader")
    ("cf3.mesh.gmsh.Reader")
    ("cf3.mesh.neu.Reader");

  boost_foreach(const std::string& reader_name, known_readers)
  {
    if(is_not_null(get_child(reader_name)))
      remove_component(reader_name);
    
    try
    {
      boost::shared_ptr<MeshReader> reader = boost::dynamic_pointer_cast<MeshReader>(build_component_nothrow(reader_name, reader_name));
      
      if(is_null(reader))
        continue;
      
      add_component(reader);
    
      boost_foreach(const std::string& extension, reader->get_extensions())
        m_extensions_to_readers[extension].push_back(reader->handle<MeshReader>());
    }
    catch(LibLoadingError& e)
    {
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void LoadMesh::load_mesh_into(const URI& file, Mesh& mesh)
{
  update_list_of_available_readers();

  const std::string extension = file.extension();

  if ( m_extensions_to_readers.count(extension) == 0 )
  {
    throw FileFormatError (FromHere(), "No meshreader exists for files with extension " + extension);
  }
  else
  {
    if (m_extensions_to_readers[extension].size()>1)
    {
      std::string msg;
      msg = file.string() + " has ambiguous extension " + extension + "\n"
        +  "Possible readers for this extension are: \n";
      boost_foreach(const Handle< MeshReader > reader , m_extensions_to_readers[extension])
        msg += " - " + reader->name() + "\n";
      throw FileFormatError( FromHere(), msg);
    }
    else
    {
      Handle< MeshReader > meshreader = m_extensions_to_readers[extension][0];
      meshreader->read_mesh_into(file, mesh);
    }
  }
}

boost::shared_ptr< Mesh > LoadMesh::load_mesh(const URI& file)
{
  boost::shared_ptr<Mesh> mesh = allocate_component<Mesh>("mesh");
  load_mesh_into(file, *mesh);
  return mesh;
}


////////////////////////////////////////////////////////////////////////////////

void LoadMesh::signal_load_mesh ( common::SignalArgs& node )
{
  update_list_of_available_readers();

  SignalOptions options( node );

  URI path = options.value<URI>("location");

  if( path.scheme() != URI::Scheme::CPATH )
    throw ProtocolError( FromHere(), "Wrong protocol to access the Parent Component, expecting a \'cpath\' but got \'" + path.string() +"\'");

  // get the domain
  Component& parent_component = *access_component( path );

  // std::vector<URI> files = option("Files").value<std::vector<URI> >();
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
    Handle<Mesh> mesh = parent_component.create_component<Mesh>(options["name"].value<std::string>());

    // Get the file paths
    boost_foreach(URI file, files)
    {
      const std::string extension = file.extension();

      if ( m_extensions_to_readers.count(extension) == 0 )
      {
        throw ValueNotFound (FromHere(), "No meshreader exists for files with extension " + extension);
        parent_component.remove_component(mesh->name());
      }
      else
      {
        if (m_extensions_to_readers[extension].size()>1)
        {
          std::string msg;
          msg = file.string() + " has ambiguous extension " + extension + "\n"
            +  "Possible readers for this extension are: \n";
          boost_foreach(const Handle< MeshReader > reader , m_extensions_to_readers[extension])
            msg += " - " + reader->name() + "\n";
          throw BadValue( FromHere(), msg);
          parent_component.remove_component(mesh->name());
        }
        else
        {
          Handle< MeshReader > meshreader = m_extensions_to_readers[extension][0];
          meshreader->read_mesh_into(file, *mesh);
        }
      }
    }
  }
  else
  {
    throw BadValue( FromHere(), "No mesh was read because no files were selected." );
  }
}

////////////////////////////////////////////////////////////////////////////////

void LoadMesh::signature_load_mesh ( common::SignalArgs& node)
{
  SignalOptions options( node );

  std::vector<URI> dummy;
  Handle< Factory > meshreader_factory = Core::instance().factories().get_factory<MeshReader>();
  std::vector<boost::any> readers;

  // build the restricted list
  boost_foreach(Builder& bdr, find_components_recursively<Builder>( *meshreader_factory ) )
  {
    readers.push_back(bdr.name());
  }

  options.add_option("location", URI() )
      .description("Path to the component to hold the mesh");

  options.add_option("name", std::string("mesh") )
      .description("Name of the mesh");

  // create de value and add the restricted list
  options.add_option( "readers", boost::any_cast<std::string>(readers[0]) )
      .description("Available readers" )
      .restricted_list() = readers ;

  options.add_option( "files", dummy )
      .description( "Files to read" );
}

} // mesh
} // cf3
