// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "common/ActionDirector.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Signal.hpp"

#include "common/PE/Comm.hpp"

#include "common/XML/SignalFrame.hpp"
#include "common/XML/SignalOptions.hpp"

#include "mesh/DiscontinuousDictionary.hpp"
#include "mesh/Faces.hpp"
#include "mesh/Region.hpp"
#include "mesh/Space.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/MeshAdaptor.hpp"
#include "mesh/Field.hpp"
#include "mesh/Functions.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/ElementData.hpp"

#include "mesh/actions/LinkPeriodicNodes.hpp"
#include "mesh/actions/MakeBoundaryGlobal.hpp"
#include "mesh/actions/RemoveGhostElements.hpp"

#include "PeriodicMeshPartitioner.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < PeriodicMeshPartitioner, MeshTransformer, mesh::actions::LibActions> PeriodicMeshPartitioner_Builder;

////////////////////////////////////////////////////////////////////////////////

namespace detail
{

Uint final_target_node(const common::List<Uint>& periodic_links_nodes, const common::List<bool>& periodic_links_active, Uint node)
{
  while(periodic_links_active[node])
  {
    node = periodic_links_nodes[node];
  }

  return node;
}

}

PeriodicMeshPartitioner::PeriodicMeshPartitioner(const std::string& name) :
  MeshTransformer(name),
  m_make_boundary_global(create_static_component<MakeBoundaryGlobal>("MakeBoundaryGlobal")),
  m_periodic_boundary_linkers(create_static_component<common::ActionDirector>("PeriodicLinks")),
  m_remove_ghosts(create_static_component<RemoveGhostElements>("RemoveGhostElements")),
  m_mesh_partitioner(create_component("MeshPartitioner", "cf3.mesh.zoltan.Partitioner"))
{
  regist_signal( "create_link_periodic_nodes" )
      .connect( boost::bind( &PeriodicMeshPartitioner::signal_create_link_periodic_nodes, this, _1 ) )
      .description("Creates a new component to link periodic nodes on both sides of a domain")
      .pretty_name("Create LinkPeriodicNodes");
}

PeriodicMeshPartitioner::~PeriodicMeshPartitioner()
{
}

void PeriodicMeshPartitioner::execute()
{
  configure_option_recursively("mesh", m_mesh);

  m_make_boundary_global->execute();
  m_periodic_boundary_linkers->execute();
  m_remove_ghosts->execute();

  // This is from the load balancer, but without overlap growing
  Mesh& mesh = *m_mesh;

  common::PE::Comm& comm = common::PE::Comm::instance();

  CFinfo << "loadbalancing mesh:" << CFendl;

  comm.barrier();
  CFinfo << "  + building joint node & element global numbering ... " << CFendl;

  // build global numbering and connectivity of nodes and elements (necessary for partitioning)
  common::build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.GlobalNumbering","glb_numbering")->transform(mesh);

  CFinfo << "  + building joint node & element global numbering ... done" << CFendl;
  comm.barrier();

  CFinfo << "  + building global node-element connectivity ... " << CFendl;

  common::build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.GlobalConnectivity","glb_connectivity")->transform(mesh);

  CFinfo << "  + building global node-element connectivity ... done" << CFendl;
  comm.barrier();

  CFinfo << "  + partitioning and migrating ..." << CFendl;
  m_mesh_partitioner->transform(mesh);
  CFinfo << "  + partitioning and migrating ... done" << CFendl;

  mesh.geometry_fields().remove_component("periodic_links_nodes");
  mesh.geometry_fields().remove_component("periodic_links_active");
  
  CFinfo << "  + building joint node & element global numbering ..." << CFendl;
  common::build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.GlobalNumbering","glb_numbering")->transform(mesh);
  CFinfo << "  + building joint node & element global numbering ... done" << CFendl;

  m_make_boundary_global->execute();
  m_periodic_boundary_linkers->execute();

  Handle< common::List<Uint> > periodic_links_nodes_h(mesh.geometry_fields().get_child("periodic_links_nodes"));
  Handle< common::List<bool> > periodic_links_active_h(mesh.geometry_fields().get_child("periodic_links_active"));
  cf3_assert(periodic_links_nodes_h);
  cf3_assert(periodic_links_active_h);

  const common::List<Uint>& periodic_links_nodes = *periodic_links_nodes_h;
  const common::List<bool>& periodic_links_active = *periodic_links_active_h;

  const Uint nb_nodes = mesh.geometry_fields().size();
  cf3_assert(nb_nodes == periodic_links_nodes.size());
  cf3_assert(nb_nodes == periodic_links_active.size());

  common::List<Uint>& ranks = mesh.geometry_fields().rank();

  for(Uint i = 0; i != nb_nodes; ++i)
  {
    ranks[i] = ranks[detail::final_target_node(periodic_links_nodes, periodic_links_active, i)];
  }

  m_remove_ghosts->execute();

  CFinfo << "  + growing overlap layer ..." << CFendl;
  common::build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.GrowOverlap","grow_overlap")->transform(mesh);
  CFinfo << "  + growing overlap layer ... done" << CFendl;

  CFinfo << "  + building joint node & element global numbering ..." << CFendl;
  common::build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.GlobalNumbering","glb_numbering")->transform(mesh);
  CFinfo << "  + building joint node & element global numbering ... done" << CFendl;

  // Make sure we finish with a valid node linking
  mesh.geometry_fields().remove_component("periodic_links_nodes");
  mesh.geometry_fields().remove_component("periodic_links_active");
  m_make_boundary_global->execute();
  m_periodic_boundary_linkers->execute();
}

Handle< MeshTransformer > PeriodicMeshPartitioner::create_link_periodic_nodes()
{
  std::stringstream new_name_str;
  new_name_str << "LinkPeriodicNodes" <<  m_periodic_boundary_linkers->count_children() + 1;
  return m_periodic_boundary_linkers->create_component<LinkPeriodicNodes>(new_name_str.str());
}

void PeriodicMeshPartitioner::signal_create_link_periodic_nodes ( common::SignalArgs& args )
{
  Handle<MeshTransformer> result = create_link_periodic_nodes();

  SignalFrame reply = args.create_reply(uri());
  SignalOptions reply_options(reply);
  reply_options.add("created_component", result->uri());
}



//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
