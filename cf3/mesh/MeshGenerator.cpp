// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/OptionURI.hpp"
#include "common/Core.hpp"
#include "common/EventHandler.hpp"

#include "common/XML/SignalOptions.hpp"

#include "mesh/MeshGenerator.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/MeshElements.hpp"

namespace cf3 {
namespace mesh {

using namespace common;
using namespace common::XML;

////////////////////////////////////////////////////////////////////////////////

MeshGenerator::MeshGenerator ( const std::string& name  ) :
  Action ( name )
{
  mark_basic();

  options().add_option(OptionURI::create("mesh","cpath:",URI::Scheme::CPATH))
      ->description("Mesh that will be generated")
      ->pretty_name("Mesh")
      ->mark_basic()
      ->attach_trigger( boost::bind( &MeshGenerator::config_mesh , this) );
}

////////////////////////////////////////////////////////////////////////////////

void MeshGenerator::config_mesh()
{
  URI mesh_uri = option("mesh").value<URI>();
  if ( Component::Ptr found_mesh = Core::instance().root().access_component_ptr( mesh_uri ) )
  {
    m_mesh = found_mesh->as_ptr_checked<Mesh>();
  }
  else
  {
    URI parent_uri = mesh_uri.base_path();
    std::string mesh_name = mesh_uri.name();
    if ( Component::Ptr found_parent = Core::instance().root().access_component_ptr( parent_uri ) )
    {
      m_mesh = found_parent->create_component_ptr<Mesh>(mesh_name);
    }
    else
    {
      throw ValueNotFound(FromHere(), "Could not find component "+parent_uri.string()+" to create mesh \""+mesh_name+"\" into");
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

Mesh& MeshGenerator::generate()
{
  execute();
  return *m_mesh.lock();
}

////////////////////////////////////////////////////////////////////////////////

void MeshGenerator::raise_mesh_loaded()
{
  Mesh& mesh = *m_mesh.lock();

  mesh.update_statistics();
  mesh.elements().update();

  // Raise an event to indicate that a mesh was loaded happened
  SignalOptions options;
  options.add_option< OptionURI >("mesh_uri", mesh.uri());

  SignalArgs f= options.create_frame();
  Core::instance().event_handler().raise_event( "mesh_loaded", f );
}

////////////////////////////////////////////////////////////////////////////////

MeshGenerator::~MeshGenerator()
{
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
