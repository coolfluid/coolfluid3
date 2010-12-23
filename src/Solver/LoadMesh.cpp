// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/Foreach.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CDomain.hpp"

#include "Solver/LoadMesh.hpp"

namespace CF {
namespace Solver {

using namespace Common;
using namespace Common::String;
using namespace CF::Mesh;

Common::ComponentBuilder < LoadMesh, Component, LibSolver > LoadMesh_Builder;

////////////////////////////////////////////////////////////////////////////////

LoadMesh::LoadMesh ( const std::string& name  ) :
  Component ( name )
{
  m_properties["brief"] = std::string("Loads meshes, guessing automatically the format from the file extension");
  mark_basic();
  
  CFactory::Ptr meshreader_factory = Core::instance().factories()->get_factory<CMeshReader>();

  if ( is_null(meshreader_factory) )
    throw ValueNotFound ( FromHere() , "Could not find factory for CMeshReader" );

  boost_foreach(CBuilder& bdr, find_components_recursively<CBuilder>( *meshreader_factory ) )
  {
    CMeshReader::Ptr reader = bdr.build(bdr.name())->as_type<CMeshReader>();
    if ( is_not_null(reader) )
    {
      add_static_component(reader);
      boost_foreach(const std::string& extension, reader->get_extensions())
        m_extensions_to_readers[extension].push_back(reader);
    }
  }

  regist_signal ( "load_mesh" , "Loads meshes, guessing automatically the format", "Load Mesh" )->connect ( boost::bind ( &LoadMesh::signal_load_mesh, this, _1 ) );

  signal("load_mesh").signature
      .insert<URI>("Path to domain", "Path to the domain to hold the mesh")
      .insert_array<URI>( "Files" , "Files to read" );

  signal("create_component").is_hidden = true;
  signal("rename_component").is_hidden = true;
  signal("delete_component").is_hidden = true;
  signal("move_component").is_hidden   = true;
}

////////////////////////////////////////////////////////////////////////////////

LoadMesh::~LoadMesh()
{
}

////////////////////////////////////////////////////////////////////////////////

void LoadMesh::signal_load_mesh ( Common::XmlNode& node )
{
  XmlParams p (node);

  URI path = p.get_option<URI>("Domain");

  if( path.scheme() != URI::Scheme::CPATH )
    throw ProtocolError( FromHere(), "Wrong protocol to access the Domain component, expecting a \'cpath\' but got \'" + path.string() +"\'");

  // get the domain
  CDomain::Ptr domain = look_component<CDomain>( path );
  if (!domain)
    throw CastingFailed( FromHere(), "Component in path \'" + path.string() + "\' is not a valid CDomain." );

  // std::vector<URI> files = property("Files").value<std::vector<URI> >();
  std::vector<URI> files = p.get_array<URI>("Files");

  // check protocol for file loading
  boost_foreach(URI file, files)
  {
    if( file.empty() || file.scheme() != URI::Scheme::FILE )
      throw ProtocolError( FromHere(), "Wrong protocol to access the file, expecting a \'file\' but got \'" + file.string() + "\'" );
  }


  // create a mesh in the domain
  if( !files.empty() )
  {
    CMesh::Ptr mesh = domain->create_component<CMesh>("Mesh");

    // Get the file paths
    boost_foreach(URI file, files)
    {
      boost::filesystem::path fpath( file.string_without_scheme() );
      const std::string extension = fpath.extension();
      
      if ( m_extensions_to_readers.count(extension) == 0 )
      {
        throw ValueNotFound (FromHere(), "No meshreader exists for files with extension " + extension);
        domain->remove_component(mesh->name());
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
          domain->remove_component(mesh->name());
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

} // Solver
} // CF
