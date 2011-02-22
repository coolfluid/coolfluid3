// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Mesh/CDomain.hpp"
#include "Mesh/LoadMesh.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

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

  regist_signal ( "load_mesh" , "Load a new mesh", "Load Mesh" )->connect ( boost::bind ( &CDomain::signal_load_mesh, this, _1 ) );
  signal("load_mesh").signature->connect( boost::bind( &CDomain::signature_load_mesh, this, _1));

  regist_signal ( "generate_mesh" , "Generate a new mesh", "Generate Mesh" )->connect ( boost::bind ( &CDomain::signal_generate_mesh, this, _1 ) );
  signal("generate_mesh").signature->connect( boost::bind( &CDomain::signature_generate_mesh, this, _1));

}

////////////////////////////////////////////////////////////////////////////////

CDomain::~CDomain()
{
}

////////////////////////////////////////////////////////////////////////////////

void CDomain::signal_load_mesh ( Common::XmlNode& node )
{
  XmlParams p(node);
  LoadMesh::Ptr mesh_loader = find_component_ptr<LoadMesh>( *Core::instance().root()->get_child("Tools") );
  CMesh::Ptr mesh = mesh_loader->load_mesh(p.get_option<URI>("File"));
  add_component(mesh);
}

////////////////////////////////////////////////////////////////////////////////

void CDomain::signature_load_mesh ( Common::XmlNode& node )
{
  XmlParams p(node);
  p.add_option<URI>("File", URI(), "Location of the file holding the mesh" );
}

////////////////////////////////////////////////////////////////////////////////


void CDomain::signal_generate_mesh ( Common::XmlNode& node )
{

}

////////////////////////////////////////////////////////////////////////////////

void CDomain::signature_generate_mesh ( Common::XmlNode& node )
{
  XmlParams p(node);
  p.add_option<std::string>("Sorry", std::string("Not implemented"), "Sorry again" );
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
