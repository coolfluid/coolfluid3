// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/Group.hpp"
#include "common/Log.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionT.hpp"
#include "common/OptionURI.hpp"
#include "common/Signal.hpp"
#include "common/FindComponents.hpp"
#include "common/EventHandler.hpp"

#include "mesh/Domain.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/LoadMesh.hpp"
#include "mesh/WriteMesh.hpp"
#include "mesh/Field.hpp"

#include "common/PE/Comm.hpp"

#include "common/XML/Protocol.hpp"
#include "common/XML/SignalOptions.hpp"

namespace cf3 {
namespace mesh {

using namespace common;
using namespace common::XML;
using namespace common::PE;

common::ComponentBuilder < Domain, Component, LibMesh > Domain_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

struct Domain::Implementation
{
  Implementation(Component& component) :
    m_component(component)
  {
  }


  void signature_load_mesh( common::SignalArgs& node )
  {
    SignalOptions options( node );

    std::vector<URI::Scheme::Type> schemes(1);
    schemes[0] = URI::Scheme::FILE;


    options.add_option< OptionURI >("file", URI() )
        ->description("Location of the file holding the mesh")
        ->cast_to<OptionURI>()->set_supported_protocols(schemes);

    options.add_option< OptionT<std::string> >("name", std::string() )
        ->description("Name for the mesh to load");
  }


  void signature_write_mesh( common::SignalArgs& node )
  {
    SignalOptions options( node );

    std::vector<URI::Scheme::Type> schemes(1);
    schemes[0] = URI::Scheme::FILE;

    options.add_option< OptionURI >("file", URI() )
        ->description("Location of the file holding the mesh")
        ->cast_to<OptionURI>()->set_supported_protocols(schemes);
  }


  Component& m_component;
  boost::weak_ptr<WriteMesh> m_write_mesh;

};

////////////////////////////////////////////////////////////////////////////////////////////

Domain::Domain( const std::string& name  ) :
  Component ( name ),
  m_implementation(new Implementation(*this))
{
  mark_basic(); // by default domains are visible

  m_properties["brief"] = std::string("Domain for a simulation");
  std::string description =
      "Holds one or more meshes.\n\n"
      "Offers signals to load or generate a mesh";
  m_properties["description"] = description;

  regist_signal( "load_mesh" )
      ->connect( boost::bind( &Domain::signal_load_mesh, this, _1 ) )
      ->description("Load a new mesh")
      ->pretty_name("Load Mesh")
      ->signature( boost::bind( &Implementation::signature_load_mesh, m_implementation.get(), _1));

  regist_signal( "write_mesh" )
      ->connect( boost::bind( &Domain::signal_write_mesh, this, _1 ) )
      ->description("Load a new mesh")
      ->pretty_name("Write Mesh")
      ->signature( boost::bind( &Implementation::signature_write_mesh, m_implementation.get(), _1));
}



Domain::~Domain() {}



Mesh& Domain::load_mesh( const URI& file, const std::string& name )
{
  Group& tools = Core::instance().tools();

  LoadMesh& mesh_loader =
      find_component<LoadMesh>( tools );

  Mesh::Ptr mesh = create_component_ptr<Mesh>(name);

  mesh_loader.load_mesh_into(file, *mesh);

  CFdebug << "Loaded mesh " << file.string() << " into mesh " << name << CFendl;

  // rebalance the mesh if necessary and create global idx and ranks

  build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.LoadBalance","load_balancer")
      ->transform(mesh);

  // raise an event to indicate that a mesh was rebalanced (changed)
  // in serial it is important still to raise the event
  // considering it like a rebalance that had no effect

  SignalOptions options;
  options.add_option< OptionURI >("mesh_uri", mesh->uri());
  options.add_option< OptionT<bool> >("mesh_rebalanced", true);
  SignalArgs args = options.create_frame();
  Core::instance().event_handler().raise_event( "mesh_changed", args);

  mesh->check_sanity();

  return *mesh;
}


/// @todo Domain writes only the first mesh. Handle multiple-mesh case.

void Domain::write_mesh(const URI& file)
{
  if(m_implementation->m_write_mesh.expired()) // created on-demand
    m_implementation->m_write_mesh = create_static_component_ptr<WriteMesh>("MeshWriter");

  std::vector<URI> state_fields;
  Mesh& mesh = find_component<Mesh>(*this);
  boost_foreach(const Field& field, find_components_recursively<Field>(mesh))
  {
    state_fields.push_back(field.uri());
  }

  m_implementation->m_write_mesh.lock()->write_mesh(mesh, file, state_fields);
}


void Domain::signal_load_mesh ( common::SignalArgs& node )
{
  SignalOptions options( node );

  URI fileuri = options.value<URI>("file");

  std::string name ("mesh");
  if( options.check("name") )
    name = options.value<std::string>("name");

  load_mesh( fileuri, name);
}



void Domain::signal_write_mesh(SignalArgs& node)
{
  SignalOptions options( node );
  URI fileuri = options.value<URI>("file");
  write_mesh(fileuri);
}


////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
