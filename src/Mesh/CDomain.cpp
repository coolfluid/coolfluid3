// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/CGroup.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/Signal.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/CDomain.hpp"
#include "Mesh/LoadMesh.hpp"
#include "Mesh/CMeshTransformer.hpp"

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

  return *mesh;
}

////////////////////////////////////////////////////////////////////////////////

void CDomain::signal_load_mesh ( Common::SignalArgs& node )
{
  SignalOptions options( node );

  URI fileuri = options.value<URI>("file");
  std::string name = options.value<std::string>("name");

  load_mesh( fileuri, name);
}

////////////////////////////////////////////////////////////////////////////////

void CDomain::signature_load_mesh ( Common::SignalArgs& node )
{
  SignalOptions options( node );

  std::vector<URI::Scheme::Type> schemes(1);
  schemes[0] = URI::Scheme::FILE;


  options.add_option< OptionURI >("file", URI() )
      ->set_description("Location of the file holding the mesh")
      ->cast_to<OptionURI>()->set_supported_protocols(schemes);

  options.add_option< OptionT<std::string> >("name", std::string() )
      ->set_description("Location of the file holding the mesh");

}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
