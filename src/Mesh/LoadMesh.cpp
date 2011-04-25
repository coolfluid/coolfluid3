// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Signal.hpp"
#include "Common/FindComponents.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/Foreach.hpp"

#include "Common/XML/SignalOptions.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CDomain.hpp"

#include "Mesh/LoadMesh.hpp"


namespace CF {
namespace Mesh {

using namespace Common;
using namespace Common::XML;
using namespace CF::Mesh;

Common::ComponentBuilder < LoadMesh, Component, LibMesh > LoadMesh_Builder;

////////////////////////////////////////////////////////////////////////////////

LoadMesh::LoadMesh ( const std::string& name  ) :
  Component ( name )
{
  // properties

  m_properties["brief"] = std::string("Loads meshes, guessing automatically the format from the file extension");
  mark_basic();

  // signals

  regist_signal ( "load_mesh" , "Loads meshes, guessing automatically the format", "Load Mesh" )->signal->connect ( boost::bind ( &LoadMesh::signal_load_mesh, this, _1 ) );

  signal("create_component")->is_hidden = true;
  signal("rename_component")->is_hidden = true;
  signal("delete_component")->is_hidden = true;
  signal("move_component")->is_hidden   = true;

  signal("load_mesh")->signature->connect(boost::bind(&LoadMesh::signature_load_mesh, this, _1));
}

LoadMesh::~LoadMesh() {}

////////////////////////////////////////////////////////////////////////////////

void LoadMesh::update_list_of_available_readers()
{
  CFactory::Ptr meshreader_factory = Core::instance().factories().get_factory<CMeshReader>();

  if ( is_null(meshreader_factory) )
    throw ValueNotFound ( FromHere() , "Could not find factory for CMeshReader" );

  m_extensions_to_readers.clear();

  boost_foreach(CBuilder& bdr, find_components_recursively<CBuilder>( *meshreader_factory ) )
  {
    CMeshReader::Ptr reader;

    Component::Ptr comp = get_child_ptr(bdr.name());
    if( is_null(comp) )
      comp = bdr.build(bdr.name());

    if( is_not_null(comp) ) // convert to reader
      reader = comp->as_ptr<CMeshReader>();
    else
      throw SetupError(FromHere(), "Builder \'" + bdr.name() + "\' failed to build the mesh reader" );

    if ( is_not_null(reader) )
    {
      add_component(reader);
      boost_foreach(const std::string& extension, reader->get_extensions())
        m_extensions_to_readers[extension].push_back(reader);
    }
    else
      throw SetupError(FromHere(), "Component with name \'" + bdr.name() + "\' is not a Mesh Reader" );
  }
}

////////////////////////////////////////////////////////////////////////////////

CMesh::Ptr LoadMesh::load_mesh(const URI& file)
{
  update_list_of_available_readers();

  boost::filesystem::path fpath( file.path() );
  const std::string extension = fpath.extension();

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
      boost_foreach(const CMeshReader::Ptr reader , m_extensions_to_readers[extension])
        msg += " - " + reader->name() + "\n";
      throw FileFormatError( FromHere(), msg);
    }
    else
    {
      CMeshReader::Ptr meshreader = m_extensions_to_readers[extension][0];
      return meshreader->create_mesh_from(fpath);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void LoadMesh::signal_load_mesh ( Common::SignalArgs& node )
{
  update_list_of_available_readers();

  SignalOptions options( node );

  URI path = options.option<URI>("Parent Component");

  if( path.scheme() != URI::Scheme::CPATH )
    throw ProtocolError( FromHere(), "Wrong protocol to access the Parent Component, expecting a \'cpath\' but got \'" + path.string() +"\'");

  // get the domain
  Component& parent_component = access_component( path );

  // std::vector<URI> files = property("Files").value<std::vector<URI> >();
  std::vector<URI> files = options.array<URI>("Files");

  // check protocol for file loading
  boost_foreach(URI file, files)
  {
    if( file.empty() || file.scheme() != URI::Scheme::FILE )
      throw ProtocolError( FromHere(), "Wrong protocol to access the file, expecting a \'file\' but got \'" + file.string() + "\'" );
  }


  // create a mesh in the domain
  if( !files.empty() )
  {
    CMesh::Ptr mesh = parent_component.create_component<CMesh>(options.option<std::string>("Name"));

    // Get the file paths
    boost_foreach(URI file, files)
    {
      boost::filesystem::path fpath( file.path() );
      const std::string extension = fpath.extension();

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
          boost_foreach(const CMeshReader::Ptr reader , m_extensions_to_readers[extension])
            msg += " - " + reader->name() + "\n";
          throw BadValue( FromHere(), msg);
          parent_component.remove_component(mesh->name());
        }
        else
        {
          CMeshReader::Ptr meshreader = m_extensions_to_readers[extension][0];
          meshreader->read_from_to(fpath, mesh);
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

void LoadMesh::signature_load_mesh ( Common::SignalArgs& node)
{
  SignalOptions options( node );

  std::vector<URI> dummy;
  CFactory::Ptr meshreader_factory = Core::instance().factories().get_factory<CMeshReader>();
  std::vector<std::string> readers;

  // build the restricted list
  boost_foreach(CBuilder& bdr, find_components_recursively<CBuilder>( *meshreader_factory ) )
  {
    readers.push_back(bdr.name());
  }

  options.add("Parent Component", URI(), "Path to the component to hold the mesh" );

  options.add<std::string>("Name", std::string("mesh"), "Name of the mesh" );

  // create de value and add the restricted list
  options.add( "Readers", std::string() , "Available readers", readers, " ; ");

  options.add("Files", dummy , " ; ", "Files to read" );
}

} // Mesh
} // CF
