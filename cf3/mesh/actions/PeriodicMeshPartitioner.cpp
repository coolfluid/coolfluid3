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

#include "mesh/ConnectivityData.hpp"
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

// Get the final target node taking into account any perodic links
Uint final_target_node(const common::List<Uint>& periodic_links_nodes, const common::List<bool>& periodic_links_active, Uint node)
{
  while(periodic_links_active[node])
  {
    node = periodic_links_nodes[node];
  }

  return node;
}

std::pair<Elements*, Uint> get_periodic_element(std::pair<Elements*, Uint> source_element)
{
  Elements& elements = *source_element.first;
  Handle< common::List<Uint> > periodic_links_elements(elements.get_child("periodic_links_elements"));
  if(is_null(periodic_links_elements)) // Elements has no periodic data
    return source_element;

  Handle<Elements> linked_elements(common::follow_link(periodic_links_elements->get_child("periodic_link")));
  cf3_assert(is_not_null(linked_elements));

  return std::make_pair(linked_elements.get(), periodic_links_elements->array()[source_element.second]);
}

Uint get_final_rank(const Uint volume_gid, std::map< const Elements*, std::vector<Uint> >& adjacent_element_gids, std::map<Uint, std::vector< std::pair<Elements*, Uint> > >& volume_to_surface_map)
{
  Uint own_rank = 0;
  const std::vector< std::pair<Elements*, Uint> >& my_surface_map = volume_to_surface_map[volume_gid];
  const Uint nb_surface_elements = my_surface_map.size();
  for(Uint i = 0; i != nb_surface_elements; ++i)
  {
    std::pair<Elements*, Uint> surface_element = my_surface_map[i];
    own_rank = surface_element.first->rank()[surface_element.second];
    std::pair<Elements*, Uint> target = get_periodic_element(surface_element);
    if(target != surface_element)
    {
      return get_final_rank(adjacent_element_gids[target.first][target.second], adjacent_element_gids, volume_to_surface_map);
    }
  }
  return own_rank;
}

} // namespace detail

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

  options().add("load_balance", true)
    .pretty_name("Load Balance")
    .description("Load balance taking into account the periodicity. If this is disabled, the existing partitioning will merely be adjusted but possibly not optimal")
    .mark_basic();
}

PeriodicMeshPartitioner::~PeriodicMeshPartitioner()
{
}

void PeriodicMeshPartitioner::execute()
{
  common::PE::Comm& comm = common::PE::Comm::instance();
  configure_option_recursively("mesh", m_mesh);
  Mesh& mesh = *m_mesh;

  if(options().value<bool>("load_balance"))
  {
    m_make_boundary_global->execute();
    m_periodic_boundary_linkers->execute();
    m_remove_ghosts->execute();

    // This is from the load balancer, but without overlap growing
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
    common::build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.GlobalConnectivity","glb_connectivity")->transform(mesh);
    CFinfo << "  + building joint node & element global numbering ... done" << CFendl;
  }
  else
  {
    m_remove_ghosts->execute();
  }

  m_make_boundary_global->execute();
  m_periodic_boundary_linkers->execute();

  boost::shared_ptr<CNodeConnectivity> node_connectivity = common::allocate_component<CNodeConnectivity>("node_connectivity");
  node_connectivity->initialize(common::find_components_recursively_with_filter<mesh::Elements>(mesh.topology(), IsElementsSurface()));
  
  // For each surface elements, a vector containing a sequence of  [surface element GID] , [adjacent volume element GID] for all volume elements on the current rank
  std::map< const Elements*, std::vector<Uint> > gids_to_send;
  
  // Create volume-to-surface connectivity and ensure each surface element has the same rank as its adjacent volume element
  BOOST_FOREACH(Elements& elements, common::find_components_recursively_with_filter<Elements>(mesh.topology(), IsElementsVolume()))
  {
    cf3_always_assert(is_null(elements.get_child("face_connectivity")));
    CFaceConnectivity& face_connectivity = *elements.create_component<CFaceConnectivity>("face_connectivity");
    face_connectivity.initialize(*node_connectivity);
    const Uint nb_elements = elements.size();
    const Uint nb_faces = elements.element_type().nb_faces();
    for(Uint elem = 0; elem != nb_elements; ++elem)
    {
      cf3_always_assert(!elements.is_ghost(elem));
      for(Uint face = 0; face != nb_faces; ++face)
      {
        if(face_connectivity.has_adjacent_element(elem, face))
        {
          CFaceConnectivity::ElementReferenceT connected = face_connectivity.adjacent_element(elem, face);
          const Elements* connected_elements = connected.first;
          const Uint connected_idx = connected.second;
          std::vector<Uint>& my_gids_to_send = gids_to_send[connected_elements];
          my_gids_to_send.push_back(connected_elements->glb_idx()[connected_idx]);
          my_gids_to_send.push_back(elements.glb_idx()[elem]);
        }
      }
    } 
  }

  // Keep track of the GID of the adjacent element for each surface element
  std::map< const Elements*, std::vector<Uint> > adjacent_element_gids;
  // Map between volume element GID and its adjacent face list
  std::map<Uint, std::vector< std::pair<Elements*, Uint> > > volume_to_surface_map;
  BOOST_FOREACH(Elements& elements, common::find_components_recursively_with_filter<Elements>(mesh.topology(), IsElementsSurface()))
  {
    std::vector< std::vector<Uint> > recv;
    comm.all_gather(gids_to_send[&elements], recv);
    
    // Create a GID-to-local index map
    std::map<Uint, Uint> gid_to_local;
    const Uint nb_elements = elements.size();
    for(Uint local_id = 0; local_id != nb_elements; ++local_id)
      gid_to_local[elements.glb_idx()[local_id]] = local_id;
    
    std::vector<Uint>& adjacent_element_gids_vec = adjacent_element_gids[&elements];
    adjacent_element_gids_vec.resize(nb_elements, std::numeric_limits<Uint>::max());
      
    cf3_assert(recv.size() == comm.size());
    const Uint nb_ranks = recv.size();
    for(Uint new_rank = 0; new_rank != nb_ranks; ++new_rank)
    {
      const std::vector<Uint>& recv_for_rank = recv[new_rank];
      cf3_assert(recv_for_rank.size() % 2 == 0);
      const Uint nb_entries = recv_for_rank.size();
      for(Uint i = 0; i != nb_entries;)
      {
        const Uint surface_gid = recv_for_rank[i++];
        const Uint volume_gid = recv_for_rank[i++];
        const Uint local_id = gid_to_local[surface_gid];
        elements.rank()[local_id] = new_rank;
        adjacent_element_gids_vec[local_id] = volume_gid;
        volume_to_surface_map[volume_gid].push_back(std::make_pair(&elements, local_id));
      }
    }
    for(Uint i = 0; i != nb_elements; ++i)
    {
      std::stringstream error_msg;
      error_msg << "No adjacent GID found for surface elements from region " << elements.parent()->name() << " with GIDs";
      bool found_error = false;
      if(adjacent_element_gids_vec[i] == std::numeric_limits<Uint>::max())
      {
        found_error = true;
        error_msg << " " << elements.glb_idx()[i];
      }
      if(found_error)
        throw common::SetupError(FromHere(), error_msg.str());
        
    }
  }

// Check that each volume element matchs with its surface elements now
#ifndef NDEBUG
  BOOST_FOREACH(Elements& elements, common::find_components_recursively_with_filter<Elements>(mesh.topology(), IsElementsVolume()))
  {
    Handle<CFaceConnectivity> face_connectivity(elements.get_child("face_connectivity"));
    cf3_assert(is_not_null(face_connectivity));
    const Uint nb_elements = elements.geometry_space().connectivity().array().size();
    const Uint nb_faces = elements.element_type().nb_faces();
    for(Uint elem = 0; elem != nb_elements; ++elem)
    {
      for(Uint face = 0; face != nb_faces; ++face)
      {
        if(face_connectivity->has_adjacent_element(elem, face))
        {
          CFaceConnectivity::ElementReferenceT connected = face_connectivity->adjacent_element(elem, face);
          cf3_assert(connected.first->rank()[connected.second] == elements.rank()[elem]);
        }
      }
    } 
  }
#endif
  
  // Compute the rank of volume elements near the surface, so that periodic boundaries are never on the boundary between two CPUs
  std::map<Uint, Uint> volume_ranks;
  for(std::map<Uint, std::vector< std::pair<Elements*, Uint> > >::const_iterator it = volume_to_surface_map.begin(); it != volume_to_surface_map.end(); ++it)
  {
    volume_ranks[it->first] = detail::get_final_rank(it->first, adjacent_element_gids, volume_to_surface_map);
  }
  
  // Adjust the ranks of the associated surface elements
  BOOST_FOREACH(mesh::Elements& elements, common::find_components_recursively_with_filter<mesh::Elements>(mesh.topology(), IsElementsSurface()))
  { 
    const Uint nb_elements = elements.size();
    for(Uint i = 0; i != nb_elements; ++i)
    {
      elements.rank()[i] = volume_ranks[adjacent_element_gids[&elements][i]];
    }
  }
#ifndef NDEBUG
  // Check that each element's periodic link is on the same rank
  BOOST_FOREACH(mesh::Elements& elements, common::find_components_recursively_with_filter<mesh::Elements>(mesh.topology(), IsElementsSurface()))
  {
    Handle< common::List<Uint> > periodic_links_elements(elements.get_child("periodic_links_elements"));
    if(is_null(periodic_links_elements)) // Elements has no periodic data
      continue;
    
    Handle<Elements const> linked_elements(common::follow_link(periodic_links_elements->get_child("periodic_link")));
    cf3_assert(is_not_null(linked_elements));
    
    const Uint nb_elements = elements.size();
    for(Uint i = 0; i != nb_elements; ++i)
    {
      cf3_assert(elements.rank()[i] == linked_elements->rank()[periodic_links_elements->array()[i]]);
    }
  }
#endif

  // Clean up the mesh structures
  mesh.geometry_fields().remove_component("periodic_links_nodes");
  mesh.geometry_fields().remove_component("periodic_links_active");
  BOOST_FOREACH(mesh::Elements& elements, common::find_components_recursively_with_filter<mesh::Elements>(mesh.topology(), IsElementsSurface()))
  {
    Handle< common::List<Uint> > periodic_links_elements(elements.get_child("periodic_links_elements"));
    if(is_not_null(periodic_links_elements)) // Clean periodic element data
      elements.remove_component("periodic_links_elements");
  }

  // Move volume elements that no longer match with the ranks of the surface elements to their surface rank
  std::vector< std::vector<std::vector<Uint> > > elements_to_move(comm.size(), std::vector<std::vector<Uint> >(mesh.elements().size()));
  BOOST_FOREACH(Elements& elements, common::find_components_recursively_with_filter<Elements>(mesh.topology(), IsElementsVolume()))
  {
    elements.remove_component("face_connectivity"); // Clean up old face connectivity
    const Uint nb_elements = elements.size();
    for(Uint elem = 0; elem != nb_elements; ++elem)
    {
      cf3_assert(elements.rank()[elem] == comm.rank());
      
      std::map<Uint,Uint>::const_iterator new_rank_it = volume_ranks.find(elements.glb_idx()[elem]);
      if(new_rank_it != volume_ranks.end() && new_rank_it->second != comm.rank())
      {
        elements_to_move[new_rank_it->second][elements.entities_idx()].push_back(elem);
      }
    }
  }

  m_remove_ghosts->execute();

  MeshAdaptor move_elements_adaptor(mesh);
  move_elements_adaptor.prepare();
  move_elements_adaptor.move_elements(elements_to_move);
  move_elements_adaptor.finish();

  CFinfo << "  + growing overlap layer ..." << CFendl;
  common::build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.GrowOverlap","grow_overlap")->transform(mesh);
  CFinfo << "  + growing overlap layer ... done" << CFendl;

  // Relink the boundaries after overlap growing
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

  // Ensure that the rank of each periodic node is the same as it's target
  for(Uint i = 0; i != nb_nodes; ++i)
  {
    ranks[i] = ranks[detail::final_target_node(periodic_links_nodes, periodic_links_active, i)];
  }

  CFdebug << "Removing unused nodes..." << CFendl;

  // Remove unused nodes
  node_connectivity = common::allocate_component<CNodeConnectivity>("node_connectivity");
  node_connectivity->initialize(common::find_components_recursively<mesh::Elements>(mesh.topology()));

  const Dictionary& geom = mesh.geometry_fields();
  Uint geom_dict_idx = 0;
  for(Uint i = 0; i != mesh.dictionaries().size(); ++i)
  {
    if(mesh.dictionaries()[i].get() == &geom)
      geom_dict_idx = i;
  }

  std::vector<bool> is_used(nb_nodes, false);
  std::vector< std::vector<Uint> > inverse_periodic_links(nb_nodes);
  // Any node connected to a volume element is used
  for(Uint i = 0; i != nb_nodes; ++i)
  {
    BOOST_FOREACH(const Uint element_glb_idx, node_connectivity->node_element_range(i))
    {
      if(node_connectivity->element(element_glb_idx).first->element_type().dimensionality() == mesh.dimension())
      {
        is_used[i] = true;
        break;
      }
    }
  }

  // Any node connected periodically to a used node is also used
  for(Uint i = 0; i != nb_nodes; ++i)
  {
    Uint target_node = i;
    while(periodic_links_active[target_node])
    {
      target_node = periodic_links_nodes[target_node];
      if(is_used[i])
        is_used[target_node] = true;
    }
    inverse_periodic_links[target_node].push_back(i);
  }

  for(Uint i = 0; i != nb_nodes; ++i)
  {
    if(!is_used[i])
      continue;
    BOOST_FOREACH(const Uint inverse_node, inverse_periodic_links[i])
    {
      is_used[inverse_node] = true;
    }
  }

  // Any element having an unused node will be removed
  MeshAdaptor remove_unused_elements(mesh);
  remove_unused_elements.prepare();
  BOOST_FOREACH(Elements& elements, common::find_components_recursively_with_filter<Elements>(mesh.topology(), IsElementsSurface()))
  {
    Connectivity& conn = elements.geometry_space().connectivity();
    const Uint nb_elements = elements.size();
    for(Uint i = 0; i != nb_elements; ++i)
    {
      BOOST_FOREACH(const Uint node_idx, conn[i])
      {
        if(!is_used[node_idx])
        {
          cf3_assert(elements.is_ghost(i));
          remove_unused_elements.remove_element(elements.entities_idx(), i);
          break;
        }
      }
    }
  }
  remove_unused_elements.finish();

  MeshAdaptor remove_nodes_adaptor(mesh);
  remove_nodes_adaptor.prepare();
  remove_nodes_adaptor.make_element_node_connectivity_global();
  for(Uint i = 0; i != nb_nodes; ++i)
  {
    if(!is_used[i])
    {
      cf3_assert(geom.is_ghost(i));
      PackedNode node(mesh, geom_dict_idx, i);
      remove_nodes_adaptor.remove_node(node);
    }
  }
  remove_nodes_adaptor.finish();

  // Finish with a clean periodic linking containing only the relevant part of the boundary
  mesh.geometry_fields().remove_component("periodic_links_nodes");
  mesh.geometry_fields().remove_component("periodic_links_active");
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
