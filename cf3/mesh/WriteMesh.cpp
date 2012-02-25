// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iomanip>

#include <boost/assign/list_of.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/replace.hpp>

#include "common/FindComponents.hpp"
#include "common/Log.hpp"
#include "common/Signal.hpp"
#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionURI.hpp"
#include "common/PropertyList.hpp"
#include "common/Core.hpp"
#include "common/Foreach.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/SignalOptions.hpp"

#include "mesh/MeshWriter.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Domain.hpp"
#include "mesh/MeshMetadata.hpp"
#include "mesh/Field.hpp"

#include "mesh/WriteMesh.hpp"

namespace cf3 {
namespace mesh {

using namespace common;
using namespace common::XML;
using namespace cf3::mesh;

common::ComponentBuilder < WriteMesh, Component, LibMesh > WriteMesh_Builder;

////////////////////////////////////////////////////////////////////////////////

WriteMesh::WriteMesh ( const std::string& name  ) :
  Action ( name )
{
  // properties

  properties()["brief"] = std::string("Write meshes, guessing automatically the format from the file extension");
  mark_basic();


  options().add_option( "mesh", m_mesh)
      .description("Mesh to write")
      .pretty_name("Mesh")
      .mark_basic()
      .link_to(&m_mesh);

  options().add_option("file", m_file)
      .supported_protocol(URI::Scheme::FILE)
      .description("File to write")
      .pretty_name("File")
      .mark_basic()
      .link_to(&m_file);

  options().add_option("fields", m_fields)
      .description("Fields to write")
      .pretty_name("Fields")
      .mark_basic()
      .link_to(&m_fields);


  // signals

  regist_signal ( "write_mesh" )
      .description( "Write mesh, guessing automatically the format" )
      .pretty_name("Write Mesh" )
      .connect ( boost::bind ( &WriteMesh::signal_write_mesh, this, _1 ) )
      .signature(boost::bind(&WriteMesh::signature_write_mesh, this, _1));

  signal("create_component")->hidden(true);
  signal("rename_component")->hidden(true);
  signal("delete_component")->hidden(true);
  signal("move_component")->hidden(true);

}

////////////////////////////////////////////////////////////////////////////////

WriteMesh::~WriteMesh()
{
}

////////////////////////////////////////////////////////////////////////////////

void WriteMesh::update_list_of_available_writers()
{
  m_extensions_to_writers.clear();
  
  // TODO proper way to find the list of potential writers
  const std::vector<std::string> known_writers = boost::assign::list_of
    ("cf3.mesh.CGNS.Writer")
    ("cf3.mesh.gmsh.Writer")
    ("cf3.mesh.neu.Writer")
    ("cf3.mesh.tecplot.Writer")
    ("cf3.mesh.VTKLegacy.Writer")
    ("cf3.mesh.VTKXML.Writer");

  boost_foreach(const std::string& writer_name, known_writers)
  {
    if(is_not_null(get_child(writer_name)))
      remove_component(writer_name);
    
    boost::shared_ptr<MeshWriter> writer = boost::dynamic_pointer_cast<MeshWriter>(build_component_nothrow(writer_name, writer_name));
    
    if(is_null(writer))
      continue;
    
    add_component(writer);
  
    boost_foreach(const std::string& extension, writer->get_extensions())
      m_extensions_to_writers[extension].push_back(writer->handle<MeshWriter>());
  }
}

////////////////////////////////////////////////////////////////////////////////

void WriteMesh::execute()
{
  if (is_null(m_mesh))
    throw SetupError (FromHere(), "mesh not set");

  if(m_fields.empty())
    write_mesh( *m_mesh, m_file ); // writes all fields
  else
    write_mesh( *m_mesh, m_file, m_fields);
}

////////////////////////////////////////////////////////////////////////////////

void WriteMesh::write_mesh( const Mesh& mesh, const URI& file )
{
  std::vector<URI> fields;

  boost_foreach( const Field& field, find_components_recursively<Field>(mesh) )
    fields.push_back(field.uri());

  write_mesh(mesh,file,fields);
}

////////////////////////////////////////////////////////////////////////////////

void WriteMesh::write_mesh( const Mesh& mesh, const URI& file, const std::vector<URI>& fields)
{
  update_list_of_available_writers();

  /// @todo this should be improved to allow http(s) which would then upload the mesh
  ///       to a remote location after writing to a temporary file
  ///       uploading can be achieved using the curl library (which we already search for in the build system)

  URI filepath = file;

  if( filepath.scheme() != URI::Scheme::FILE )
    filepath.scheme( URI::Scheme::FILE );


  const std::string extension = filepath.extension();

  if ( m_extensions_to_writers.count(extension) == 0 )
    throw FileFormatError (FromHere(), "No meshwriter exists for files with extension " + extension);

  if (m_extensions_to_writers[extension].size()>1)
  {
     std::string msg;
     msg = filepath.string() + " has ambiguous extension " + extension + "\n"
       +  "Possible writers for this extension are: \n";
     boost_foreach(const Handle< MeshWriter > writer , m_extensions_to_writers[extension])
       msg += " - " + writer->name() + "\n";
     throw FileFormatError( FromHere(), msg);
   }

  // substitute the regex wildcards in the file name

  const MeshMetadata& metadata = mesh.metadata();

  std::string file_str = filepath.path();
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

  // change the path in the filepath

  filepath.path( file_str );

  // get the correct writer based on the extension

  Handle< MeshWriter > writer = m_extensions_to_writers[extension][0];
  writer->options().configure_option("fields",fields);
  writer->options().configure_option("mesh",mesh.handle<Mesh>());
  writer->options().configure_option("file", filepath);

  writer->execute();

  CFinfo << "wrote mesh in file " << filepath.string() << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

void WriteMesh::signal_write_mesh ( common::SignalArgs& node )
{
  SignalOptions options( node );

  update_list_of_available_writers();

  URI mesh_uri = options.value<URI>("mesh");

  // get the mesh
  const Mesh& mesh = *Handle<Mesh>(access_component(mesh_uri));

  const URI file = options.value<URI>("file");

  const std::vector<URI> fields;

  write_mesh(mesh,file,fields);
}

////////////////////////////////////////////////////////////////////////////////

void WriteMesh::signature_write_mesh ( common::SignalArgs& node)
{
  SignalOptions options( node );

  Handle< Factory > meshwriter_factory = Core::instance().factories().get_factory<MeshWriter>();
  std::vector<boost::any> writers;

  // build the restricted list
  boost_foreach(Builder& bdr, find_components_recursively<Builder>( *meshwriter_factory ) )
  {
    writers.push_back(bdr.name());
  }

  options.add_option("mesh", URI() )
      .supported_protocol( URI::Scheme::CPATH )
      .description("Path to the mesh");

  // create the value and add the restricted list
  options.add_option( "Available writers", std::string() )
      .description("Available writers")
      .restricted_list() = writers;

  options.add_option("file", std::string() )
      .description("File to write");
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
