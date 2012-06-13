// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "coolfluid-packages.hpp"

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

#include "common/XML/SignalOptions.hpp"

#include "mesh/MeshAdaptor.hpp"
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

  // options

  options().add("dimension", 0u)
      .description("The coordinate dimension (0 --> maximum dimensionality)")
      .pretty_name("Dimension");

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
  #ifdef CF3_HAVE_CGNS
    ("cf3.mesh.CGNS.Reader")
  #endif
    ("cf3.mesh.gmsh.Reader")
    ("cf3.mesh.neu.Reader");

  boost_foreach(const std::string& reader_name, known_readers)
  {
    if(is_not_null(get_child(reader_name)))
      remove_component(reader_name);
  
    boost::shared_ptr<MeshReader> reader = boost::dynamic_pointer_cast<MeshReader>(build_component_nothrow(reader_name, reader_name));
    
    if(is_null(reader))
      continue;
    
    add_component(reader);
  
    boost_foreach(const std::string& extension, reader->get_extensions())
      m_extensions_to_readers[extension].push_back(reader->handle<MeshReader>());
  }
}

////////////////////////////////////////////////////////////////////////////////


void LoadMesh::load_multiple_files(const std::vector<URI>& files, Mesh& mesh)
{
  if (files.size() == 0)
  {
    throw SetupError(FromHere(),"No files are specified to be loaded");
  }

  if (files.size() == 1 && mesh.dimension()==0)
  {
    load_mesh_into(files[0],mesh);
  }
  else // files need to be combined
  {
    MeshAdaptor mesh_adaptor(mesh);

    for (Uint f=0; f<files.size(); ++f)
    {
      const URI& file = files[f];
      boost::shared_ptr<Mesh> tmp_mesh = allocate_component<Mesh>(file.base_name());
      load_mesh_into(file,*tmp_mesh);
      mesh_adaptor.combine_mesh(*tmp_mesh);
    }
    mesh_adaptor.remove_duplicate_elements_and_nodes();
    mesh_adaptor.fix_node_ranks();
    mesh_adaptor.finish();
  }
}


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
      meshreader->options().set("mesh",mesh.handle<Mesh>());
      meshreader->options().set("file",file);
      meshreader->options().set("dimension",options().value<Uint>("dimension"));
      meshreader->execute();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void LoadMesh::signal_load_mesh ( common::SignalArgs& node )
{
  update_list_of_available_readers();

  SignalOptions options( node );

  URI path = options.value<URI>("mesh");

  if( path.scheme() != URI::Scheme::CPATH )
    throw ProtocolError( FromHere(), "Wrong protocol to access the Parent Component, expecting a \'cpath\' but got \'" + path.string() +"\'");

  // std::vector<URI> files = option("Files").value<std::vector<URI> >();
  std::vector<URI> files = options.array<URI>("files");

  // check protocol for file loading
  boost_foreach(URI file, files)
  {
    if( file.empty() || file.scheme() != URI::Scheme::FILE )
      throw ProtocolError( FromHere(), "Wrong protocol to access the file, expecting a \'file\' but got \'" + file.string() + "\'" );
  }

  // create a mesh in the domain
  if( files.empty() )
  {
    throw BadValue( FromHere(), "No mesh was read because no files were selected." );
  }

  Handle<Mesh> mesh;
  if (Handle<Component> found_mesh = access_component(path))
  {
    mesh = found_mesh->handle<Mesh>();
  }
  else
  {
    Handle<Component> parent = access_component(path.base_path());
    if (is_null(parent))
      throw SetupError(FromHere(),"Component "+path.base_path().string()+" does dont exist");
    mesh = parent->create_component<Mesh>(path.base_name());
  }

  load_multiple_files(files,*mesh);

  SignalFrame reply = node.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", mesh->uri());
}

////////////////////////////////////////////////////////////////////////////////

void LoadMesh::signature_load_mesh ( common::SignalArgs& node)
{
  SignalOptions options( node );

  options.add("mesh", Core::instance().root().uri()/URI("./mesh") )
      .description("Path to the mesh component. Mesh will be created if doesn't exist");

  options.add( "files", std::vector<URI>() )
      .description( "Files to load into one single mesh" );
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
