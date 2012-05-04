// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "common/FindComponents.hpp"
#include "common/List.hpp"
#include "common/PE/Comm.hpp"

#include "mesh/Region.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Functions.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/Space.hpp"

#include "UFEM/SparsityBuilder.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace mesh;

////////////////////////////////////////////////////////////////////////////////

boost::shared_ptr< List<Uint> > build_sparsity(const std::vector< Handle<Region> >& regions, const Dictionary& dictionary, std::vector<Uint>& node_connectivity, std::vector<Uint>& start_indices, List<Uint>& gids, List<Uint>& ranks, List<Uint>& used_node_map)
{
  // Get some data from the dictionary
  const Uint nb_global_nodes = dictionary.size();
  const List<Uint>& dict_gid = dictionary.glb_idx();
  const List<Uint>& dict_rank = dictionary.rank();

  const Uint my_rank = PE::Comm::instance().rank();
  const Uint nb_procs = PE::Comm::instance().size();

  // Build a list of used entities
  std::vector< Handle<Entities const> > used_entities;
  BOOST_FOREACH(const Handle<Region>& region, regions)
  {
    BOOST_FOREACH(const Entities& entities, find_components_recursively_with_filter<Entities>(*region, IsElementsVolume()))
    {
      used_entities.push_back(entities.handle<Entities>());
    }
  }

  // Build used node list, together with a mapping from old node ID to ID in the node list, as well as the new GIDs
  boost::shared_ptr< List<Uint> > used_nodes_ptr = build_used_nodes_list(used_entities, dictionary, true);
  const List<Uint>& used_nodes = *used_nodes_ptr;
  const Uint nb_used_nodes = used_nodes.size();
  gids.resize(nb_used_nodes);
  ranks.resize(nb_used_nodes);
  used_node_map.resize(nb_global_nodes);
  Uint nb_local_nodes = 0;
  for(Uint i = 0; i != nb_used_nodes; ++i)
  {
    const Uint node_idx = used_nodes[i];
    used_node_map[node_idx] = i;
    if(my_rank == dict_rank[node_idx])
    {
      ++nb_local_nodes;
    }
    ranks[i] = dict_rank[i];
  }

  // Get the layout of the new GIDs across CPUs
  std::vector<Uint> gid_distribution; gid_distribution.reserve(nb_procs);
  if(PE::Comm::instance().is_active())
  {
    // Get the total number of elements on each rank
    PE::Comm::instance().all_gather(nb_local_nodes, gid_distribution);
  }
  else
  {
    gid_distribution.push_back(nb_local_nodes);
  }
  cf3_assert(gid_distribution.size() == nb_procs);
  for(Uint i = 1; i != nb_procs; ++i)
    gid_distribution[i] += gid_distribution[i-1];

  // first gid on this rank
  Uint gid_counter = my_rank == 0 ? 0 : gid_distribution[my_rank-1];
  // copy of the GIDs, where the used node GID will be replaced by the new GID
  std::vector<Uint> replaced_gids(dict_gid.array().begin(), dict_gid.array().end());

  // For each rank, the indices that need to be received from the GID list
  std::vector< std::vector<Uint> > gids_to_receive(nb_procs);
  std::vector< std::vector<Uint> > lids_to_receive(nb_procs);
  std::vector< std::vector<Uint> > gids_to_send(nb_procs);

  // Fill gid list
  for(Uint i = 0; i != nb_used_nodes; ++i)
  {
    if(ranks[i] == my_rank)
    {
      gids[i] = gid_counter++;
      replaced_gids[used_nodes[i]] = gids[i];
    }
    else
    {
      lids_to_receive[ranks[i]].push_back(used_nodes[i]);
      gids_to_receive[ranks[i]].push_back(dict_gid[used_nodes[i]]);
    }
  }

  if(PE::Comm::instance().is_active())
  {
    // Let all ranks know which items of the GID vector need to be updated
    PE::Comm::instance().all_to_all(gids_to_receive, gids_to_send);
    cf3_assert(gids_to_send.size() == nb_procs);
    std::vector<int> send_num(nb_procs, 0);
    std::vector<int> recv_num(nb_procs, 0);
    int send_size = 0;
    int recv_size = 0;

    for(Uint i = 0; i != nb_procs; ++i)
    {
      send_num[i] = gids_to_send[i].size();
      recv_num[i] = gids_to_receive[i].size();
      send_size += send_num[i];
      recv_size += recv_num[i];
    }

    std::vector<int> recv_map; recv_map.reserve(recv_size);
    std::vector<int> send_map; send_map.reserve(send_size);
    
    std::map<Uint, Uint> gids_reverse_map;
    for(Uint i = 0; i != nb_global_nodes; ++i)
      gids_reverse_map[dict_gid[i]] = i;

    for(Uint i = 0; i != nb_procs; ++i)
    {
      recv_map.insert(recv_map.end(), lids_to_receive[i].begin(), lids_to_receive[i].end());
      const std::vector<Uint> send_gids_i = gids_to_send[i];
      const Uint len_send_gids_i = send_gids_i.size();
      for(Uint j = 0; j != len_send_gids_i; ++j)
        send_map.push_back(gids_reverse_map[send_gids_i[j]]);
    }
    
    // Update the GIDs for the ghosts
    PE::Comm::instance().all_to_all(replaced_gids, send_num, send_map, replaced_gids, recv_num, recv_map);

    for(Uint i = 0; i != nb_used_nodes; ++i)
    {
      if(ranks[i] != my_rank)
      {
        gids[i] = replaced_gids[used_nodes[i]];
      }
    }
  }

  std::vector< std::set<Uint> > connectivity_sets(nb_used_nodes);
  start_indices.assign(nb_used_nodes+1, 0);

  // Determine the number of connected nodes for each element
  BOOST_FOREACH(const Handle<Entities const>& elements, used_entities)
  {
    const Connectivity& connectivity = elements->geometry_space().connectivity();
    const Uint nb_elems = connectivity.size();
    const Uint nb_elem_nodes = connectivity.row_size();
    for(Uint elem = 0; elem != nb_elems; ++elem)
    {
      BOOST_FOREACH(const Uint node_a, connectivity[elem])
      {
        BOOST_FOREACH(const Uint node_b, connectivity[elem])
        {
          connectivity_sets[used_node_map[node_a]].insert(used_node_map[node_b]);
        }
      }
    }
  }

  // Sum the number of connected nodes to get the real start indices
  const Uint start_indices_end = start_indices.size();
  for(Uint i = 1; i != start_indices_end; ++i)
  {
    start_indices[i] = connectivity_sets[i-1].size() + start_indices[i-1];
  }

  node_connectivity.reserve(start_indices.back());
  for(Uint node = 0; node != nb_used_nodes; ++node)
  {
    node_connectivity.insert(node_connectivity.begin() + start_indices[node], connectivity_sets[node].begin(), connectivity_sets[node].end());
  }

  return used_nodes_ptr;
}


////////////////////////////////////////////////////////////////////////////////

} // UFEM
} // cf3
