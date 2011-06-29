// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iomanip>

#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>

#include "Common/Signal.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionURI.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/Foreach.hpp"

#include "Common/XML/Protocol.hpp"
#include "Common/XML/SignalOptions.hpp"

#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CDomain.hpp"
#include "Mesh/MeshMetadata.hpp"

#include "Mesh/WriteMesh.hpp"

namespace CF {
namespace Mesh {

using namespace Common;
using namespace Common::XML;
using namespace CF::Mesh;

Common::ComponentBuilder < WriteMesh, Component, LibMesh > WriteMesh_Builder;

////////////////////////////////////////////////////////////////////////////////

WriteMesh::WriteMesh ( const std::string& name  ) :
  CAction ( name )
{
  // properties

  m_properties["brief"] = std::string("Write meshes, guessing automatically the format from the file extension");
  mark_basic();


  m_options.add_option( OptionComponent<CMesh>::create("mesh","Mesh","Mesh to write",&m_mesh) )
      ->mark_basic();

  m_options.add_option( OptionURI::create("file","File","File to write",m_file,URI::Scheme::FILE) )
      ->mark_basic()
      ->link_to(&m_file);

  m_options.add_option( OptionArrayT<URI>::create("fields","Fields","Fields to write",m_fields) )
      ->mark_basic()
      ->link_to(&m_fields);


  // signals

  regist_signal ( "write_mesh" , "Write mesh, guessing automatically the format", "Write Mesh" )->signal->connect ( boost::bind ( &WriteMesh::signal_write_mesh, this, _1 ) );

  signal("create_component")->is_hidden = true;
  signal("rename_component")->is_hidden = true;
  signal("delete_component")->is_hidden = true;
  signal("move_component")->is_hidden   = true;

  signal("write_mesh")->signature->connect(boost::bind(&WriteMesh::signature_write_mesh, this, _1));
}

////////////////////////////////////////////////////////////////////////////////

WriteMesh::~WriteMesh()
{
}

////////////////////////////////////////////////////////////////////////////////

void WriteMesh::update_list_of_available_writers()
{
  CFactory::Ptr meshwriter_factory = Core::instance().factories().get_factory<CMeshWriter>();

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
    {
      writer = comp->as_ptr_checked<CMeshWriter>();
    }
    else
      throw SetupError(FromHere(), "Builder \'" + bdr.name() + "\' failed to build the mesh writer" );

    if ( is_not_null(writer) )
    {
      boost_foreach(const std::string& extension, writer->get_extensions())
        m_extensions_to_writers[extension].push_back(writer);

      if ( is_null(get_child_ptr(writer->name())))
        add_component(writer);
    }
    else
      throw SetupError(FromHere(), "Component with name \'" + bdr.name() + "\' is not a Mesh Reader" );
  }
}

////////////////////////////////////////////////////////////////////////////////

void WriteMesh::execute()
{
  if (m_mesh.expired()) throw SetupError (FromHere(), "mesh not set");
  write_mesh(*m_mesh.lock(),m_file,m_fields);
}

////////////////////////////////////////////////////////////////////////////////

void WriteMesh::write_mesh( const CMesh& mesh, const URI& file, const std::vector<URI>& fields)
{
  update_list_of_available_writers();

  const std::string extension = file.extension();

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

  // Check for environment variables
  const MeshMetadata& metadata = mesh.metadata();
  std::string file_str = file.string();
  boost::regex re("\\$\\{(\\w+)\\}");
  boost::sregex_iterator itr(file_str.begin(), file_str.end(), re);
  boost::sregex_iterator end;
  std::vector<std::pair<std::string,std::string> > matches;
  for(; itr!=end; ++itr)
  {
    matches.push_back( std::make_pair((*itr)[0],(*itr)[1]) );
  }

  for (Uint i=0; i<matches.size(); ++i)
  {
    if ( metadata.check(matches[i].second) )
    {
      std::string replace_str;
      if (matches[i].second == "iter")
      {
        std::stringstream ss;
        ss << std::setw( 4 ) << std::setfill( '0' ) << metadata.properties().value<Uint>(matches[i].second);
        replace_str = ss.str();
      }
      else if (matches[i].second == "time")
      {
        std::stringstream ss;
        ss << std::setprecision(4) << std::setiosflags(std::ios_base::scientific) << std::setw(10) << metadata.properties().value<Real>(matches[i].second);
        replace_str = ss.str();
      }
      else
      {
        replace_str = metadata.properties().value_str(matches[i].second);
      }
      boost::algorithm::replace_first(file_str,matches[i].first,replace_str);
    }
  }

  CMeshWriter::Ptr writer = m_extensions_to_writers[extension][0];
  writer->configure_option("fields",fields);
  return writer->write_from_to(mesh,URI(file_str));
}

////////////////////////////////////////////////////////////////////////////////

void WriteMesh::signal_write_mesh ( Common::SignalArgs& node )
{
  SignalOptions options( node );

  update_list_of_available_writers();

  URI mesh_uri = options.option<URI>("mesh");

  // get the mesh
  const CMesh& mesh = access_component( mesh_uri ).as_type<CMesh>();

  const URI file = options.option<URI>("file");

  const std::vector<URI> fields;

  write_mesh(mesh,file,fields);
}

////////////////////////////////////////////////////////////////////////////////

void WriteMesh::signature_write_mesh ( Common::SignalArgs& node)
{
  SignalOptions options( node );

  CFactory::Ptr meshwriter_factory = Core::instance().factories().get_factory<CMeshWriter>();
  std::vector<std::string> writers;
  std::vector<URI::Scheme::Type> schemes(1);
  schemes[0] = URI::Scheme::CPATH;

  // build the restricted list
  boost_foreach(CBuilder& bdr, find_components_recursively<CBuilder>( *meshwriter_factory ) )
  {
    writers.push_back(bdr.name());
  }

  options.add("mesh", URI(), "Path to the mesh", schemes );

  // create the value and add the restricted list
  options.add( "Available writers", std::string() , "Available writers", writers, " ; ");

  options.add<std::string>("file", std::string() , "File to write" );
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
