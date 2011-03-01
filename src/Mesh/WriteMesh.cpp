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

#include "Common/XML/Protocol.hpp"

#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CDomain.hpp"

#include "Mesh/WriteMesh.hpp"

namespace CF {
namespace Mesh {

using namespace Common;
using namespace Common::XML;
using namespace CF::Mesh;

Common::ComponentBuilder < WriteMesh, Component, LibMesh > WriteMesh_Builder;

////////////////////////////////////////////////////////////////////////////////

WriteMesh::WriteMesh ( const std::string& name  ) :
  Component ( name )
{
  // properties

  m_properties["brief"] = std::string("Write meshes, guessing automatically the format from the file extension");
  mark_basic();

  // signals

  regist_signal ( "write_mesh" , "Write mesh, guessing automatically the format", "Write Mesh" )->connect ( boost::bind ( &WriteMesh::signal_write_mesh, this, _1 ) );

  signal("create_component").is_hidden = true;
  signal("rename_component").is_hidden = true;
  signal("delete_component").is_hidden = true;
  signal("move_component").is_hidden   = true;

  update_list_of_available_writers();

  signal("write_mesh").signature->connect(boost::bind(&WriteMesh::signature_write_mesh, this, _1));
}

////////////////////////////////////////////////////////////////////////////////

WriteMesh::~WriteMesh()
{
}

////////////////////////////////////////////////////////////////////////////////

void WriteMesh::update_list_of_available_writers()
{
  CFactory::Ptr meshwriter_factory = Core::instance().factories()->get_factory<CMeshWriter>();

  if ( is_null(meshwriter_factory) )
    throw ValueNotFound ( FromHere() , "Could not find factory for CMeshWriter" );

  m_extensions_to_writers.clear();

  boost_foreach(CBuilder& bdr, find_components_recursively<CBuilder>( *meshwriter_factory ) )
  {
    CMeshWriter::Ptr writer;

    Component::Ptr comp = get_child_ptr(bdr.name());
    if( is_null(comp) )
      comp = bdr.build(bdr.name());

    if( is_not_null(comp) ) // convert to writer
      writer = comp->as_ptr<CMeshWriter>();
    else
      throw SetupError(FromHere(), "Builder \'" + bdr.name() + "\' failed to build the mesh writer" );

    if ( is_not_null(writer) )
    {
      add_component(writer);
      boost_foreach(const std::string& extension, writer->get_extensions())
        m_extensions_to_writers[extension].push_back(writer);
    }
    else
      throw SetupError(FromHere(), "Component with name \'" + bdr.name() + "\' is not a Mesh Reader" );
  }
}

////////////////////////////////////////////////////////////////////////////////

void WriteMesh::write_mesh( CMesh& mesh, const URI& file, const std::vector<URI>& fields)
{
  update_list_of_available_writers();

  boost::filesystem::path fpath( file.path() );
  const std::string extension = fpath.extension();

  if ( m_extensions_to_writers.count(extension) == 0 )
    throw FileFormatError (FromHere(), "No meshwriter exists for files with extension " + extension);

  if (m_extensions_to_writers[extension].size()>1)
  {
     std::string msg;
     msg = file.string() + " has ambiguous extension " + extension + "\n"
       +  "Possible writers for this extension are: \n";
     boost_foreach(const CMeshWriter::Ptr writer , m_extensions_to_writers[extension])
       msg += " - " + writer->name() + "\n";
     throw FileFormatError( FromHere(), msg);
   }

  CMeshWriter::Ptr writer = m_extensions_to_writers[extension][0];
  writer->configure_property("Fields",fields);
  return writer->write_from_to(mesh.as_ptr<CMesh>(),fpath);
}

////////////////////////////////////////////////////////////////////////////////

void WriteMesh::signal_write_mesh ( Common::Signal::arg_t& node )
{
  SignalFrame & options = node.map( Protocol::Tags::key_options() );

  update_list_of_available_writers();

  URI mesh_uri = options.get_option<URI>("Mesh");

  if( mesh_uri.scheme() != URI::Scheme::CPATH )
    throw ProtocolError( FromHere(), "Wrong protocol to access the Mesh, expecting a \'cpath\' but got \'" + mesh_uri.string() +"\'");

  // get the domain
  CMesh::Ptr mesh = access_component_ptr( mesh_uri )->as_ptr<CMesh>();

  std::string file = options.get_option<std::string>("File");

  // check protocol for file loading
  // if( file.scheme() != URI::Scheme::FILE )
  //   throw ProtocolError( FromHere(), "Wrong protocol to access the file, expecting a \'file\' but got \'" + file.string() + "\'" );

  URI fpath( file );
//  if( fpath.scheme() != URI::Scheme::FILE )
//    throw ProtocolError( FromHere(), "Wrong protocol to access the file, expecting a \'file\' but got \'" + fpath.string() + "\'" );

  write_mesh(*mesh,fpath,std::vector<URI>());
}

////////////////////////////////////////////////////////////////////////////////

void WriteMesh::signature_write_mesh ( Common::Signal::arg_t& node)
{
  SignalFrame & options = node.map( Protocol::Tags::key_options() );

  CFactory::Ptr meshwriter_factory = Core::instance().factories()->get_factory<CMeshWriter>();
  std::vector<std::string> writers;

  // build the restricted list
  boost_foreach(CBuilder& bdr, find_components_recursively<CBuilder>( *meshwriter_factory ) )
  {
    writers.push_back(bdr.name());
  }

  options.set_option<URI>("Mesh", URI(), "Path to the mesh" );

  // create the value and add the restricted list
  XmlNode wrts_node = options.set_option( "Available writers", std::string() , "Available writers" );
  Map(wrts_node).set_array( Protocol::Tags::key_restricted_values(), writers, " ; ");

  options.set_option<std::string>("File", std::string() , "File to write" );
}

} // Mesh
} // CF
