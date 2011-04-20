// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/Signal.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/CDomain.hpp"
#include "Mesh/LoadMesh.hpp"

#include "Common/XML/Protocol.hpp"
#include "Common/XML/SignalOptions.hpp"

namespace CF {
namespace Mesh {

using namespace Common;
using namespace Common::XML;

Common::ComponentBuilder < CDomain, Component, LibMesh > CDomain_Builder;

////////////////////////////////////////////////////////////////////////////////

CDomain::CDomain( const std::string& name  ) : Component ( name )
{
  mark_basic(); // by default domains are visible

  m_properties["brief"] = std::string("Domain for a simulation");
  std::string description =
   "Holds one or more meshes.\n\n"
   "Offers signals to load or generate a mesh";
  m_properties["description"] = description;

  regist_signal ( "load_mesh" , "Load a new mesh", "Load Mesh" )->signal->connect ( boost::bind ( &CDomain::signal_load_mesh, this, _1 ) );
  signal("load_mesh")->signature->connect( boost::bind( &CDomain::signature_load_mesh, this, _1));

}

////////////////////////////////////////////////////////////////////////////////

CDomain::~CDomain()
{
}

////////////////////////////////////////////////////////////////////////////////

void CDomain::signal_load_mesh ( Common::SignalArgs& node )
{
  SignalOptions options( node );

  LoadMesh& mesh_loader = find_component<LoadMesh>( Core::instance().root().get_child("Tools") );
  CMesh::Ptr mesh = mesh_loader.load_mesh(options.option<URI>("File"));
  mesh->rename(options.option<std::string>("Name"));
  add_component(mesh);
}

////////////////////////////////////////////////////////////////////////////////

void CDomain::signature_load_mesh ( Common::SignalArgs& node )
{
  SignalOptions options( node );

  std::vector<URI::Scheme::Type> schemes(1);
  schemes[0] = URI::Scheme::FILE;

  options.add("File", URI(), "Location of the file holding the mesh", schemes );
  options.add<std::string>("Name", std::string(), "Location of the file holding the mesh" );
  
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
