// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/CGroup.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/Signal.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/CDomain.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/LoadMesh.hpp"
#include "Mesh/WriteMesh.hpp"

#include "Common/MPI/PE.hpp"

#include "Common/XML/Protocol.hpp"
#include "Common/XML/SignalOptions.hpp"

namespace CF {
namespace Mesh {

using namespace Common;
using namespace Common::XML;
using namespace Common::mpi;

Common::ComponentBuilder < CDomain, Component, LibMesh > CDomain_Builder;

////////////////////////////////////////////////////////////////////////////////

struct CDomain::Implementation
{
  Implementation(Component& component) :
    m_component(component)
  {
    m_component.options().add_option(OptionComponent<CMesh>::create("active_mesh", &m_active_mesh))
      ->set_pretty_name("Active Mesh")
      ->set_description("Root region below which the boundary conditions are defined");
  }
  
  void signature_load_mesh( Common::SignalArgs& node )
  {
      SignalOptions options( node );

  std::vector<URI::Scheme::Type> schemes(1);
  schemes[0] = URI::Scheme::FILE;


  options.add_option< OptionURI >("file", URI() )
      ->set_description("Location of the file holding the mesh")
      ->cast_to<OptionURI>()->set_supported_protocols(schemes);

  options.add_option< OptionT<std::string> >("name", std::string() )
      ->set_description("Name for the mesh to load");
  }
  
  void signature_write_mesh( Common::SignalArgs& node )
  {
    SignalOptions options( node );

    std::vector<URI::Scheme::Type> schemes(1);
    schemes[0] = URI::Scheme::FILE;

    options.add_option< OptionURI >("file", URI() )
      ->set_description("Location of the file holding the mesh")
      ->cast_to<OptionURI>()->set_supported_protocols(schemes);
  }
  
  Component& m_component;
  boost::weak_ptr<WriteMesh> m_write_mesh;
  boost::weak_ptr<CMesh> m_active_mesh;
};

////////////////////////////////////////////////////////////////////////////////

CDomain::CDomain( const std::string& name  ) :
  Component ( name ),
  m_implementation(new Implementation(*this))
{
  mark_basic(); // by default domains are visible

  m_properties["brief"] = std::string("Domain for a simulation");
  std::string description =
   "Holds one or more meshes.\n\n"
   "Offers signals to load or generate a mesh";
  m_properties["description"] = description;

  regist_signal ( "load_mesh" , "Load a new mesh", "Load Mesh" )->signal->connect ( boost::bind ( &CDomain::signal_load_mesh, this, _1 ) );
  signal("load_mesh")->signature->connect( boost::bind( &Implementation::signature_load_mesh, m_implementation.get(), _1));
  
  regist_signal ( "write_mesh" , "Load a new mesh", "Load Mesh" )->signal->connect ( boost::bind ( &CDomain::signal_write_mesh, this, _1 ) );
  signal("write_mesh")->signature->connect( boost::bind( &Implementation::signature_write_mesh, m_implementation.get(), _1));
}

////////////////////////////////////////////////////////////////////////////////

CDomain::~CDomain()
{
}

////////////////////////////////////////////////////////////////////////////////

CMesh& CDomain::load_mesh( const URI& file, const std::string& name )
{
  CGroup& tools = Core::instance().tools();

  LoadMesh& mesh_loader =
      find_component<LoadMesh>( tools );

  CMesh::Ptr mesh = mesh_loader.load_mesh(file);
  mesh->rename(name);
  add_component(mesh);

  // rebalance the mesh if necessary and create global idx and ranks
// not working ? //   tools.get_child("LoadBalancer").as_type<CMeshTransformer>().transform( mesh );

  build_component_abstract_type<CMeshTransformer>("CF.Mesh.Actions.LoadBalance","load_balancer")->transform(mesh);
  
  if(m_implementation->m_active_mesh.expired()) // If this is the first mesh loaded, make it active
    set_active_mesh(*mesh);

  return *mesh;
}

void CDomain::write_mesh(const URI& file)
{
  if(m_implementation->m_write_mesh.expired()) // created on-demand
    m_implementation->m_write_mesh = create_static_component_ptr<WriteMesh>("MeshWriter");
  
  std::vector<URI> state_fields;
  CMesh& mesh = active_mesh();
  boost_foreach(const CField& field, find_components_recursively<CField>(mesh))
  {
    state_fields.push_back(field.uri());
  }
  m_implementation->m_write_mesh.lock()->write_mesh(mesh, file, state_fields);
}

void CDomain::set_active_mesh(CMesh& mesh)
{
  configure_option("active_mesh", mesh.as_ptr<CMesh>());
}

CMesh& CDomain::active_mesh()
{
  if(m_implementation->m_active_mesh.expired())
    throw ValueNotFound(FromHere(), "No active mesh set for domain " + uri().string());
  return *m_implementation->m_active_mesh.lock();
}

////////////////////////////////////////////////////////////////////////////////

void CDomain::signal_load_mesh ( Common::SignalArgs& node )
{
  SignalOptions options( node );

  URI fileuri = options.value<URI>("file");
  std::string name = options.value<std::string>("name");

  load_mesh( fileuri, name);
}

void CDomain::signal_write_mesh(SignalArgs& node)
{
  SignalOptions options( node );
  URI fileuri = options.value<URI>("file");
  write_mesh(fileuri);
}


////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
