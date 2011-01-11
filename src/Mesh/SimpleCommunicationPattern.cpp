// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/mpi/collectives.hpp>

#include "Common/MPI/PE.hpp"
#include "Common/Log.hpp"

#include "Mesh/SimpleCommunicationPattern.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CNodes.hpp"

namespace CF {
namespace Mesh {

using namespace Common;
  
SimpleCommunicationPattern::SimpleCommunicationPattern()  : 
    receive_dist(mpi::PE::instance().size()+1, 0),
    send_dist(mpi::PE::instance().size()+1, 0)
{
}

void SimpleCommunicationPattern::update_send_lists()
{
  boost::mpi::communicator world;
  const Uint nb_procs = mpi::PE::instance().size();
  
  IndicesT receive_counts(nb_procs);
  IndicesT send_counts(nb_procs);
  
  for(Uint i = 0; i != nb_procs; ++i)
    receive_counts[i] = receive_dist[i+1] - receive_dist[i];
  
  boost::mpi::all_to_all(world, receive_counts, send_counts);
  
  for(Uint i = 0; i != nb_procs; ++i)
    send_dist[i+1] = send_dist[i] + send_counts[i];
  
  send_list.resize(send_dist.back());
  
  
  std::vector<boost::mpi::request> reqs;
  reqs.reserve(nb_procs*2);
  for(Uint i = 0; i != nb_procs; ++i)
  {
    reqs.push_back(world.isend(i, 0, &receive_list[receive_dist[i]], receive_dist[i+1]-receive_dist[i]));
    reqs.push_back(world.irecv(i, 0, &send_list[send_dist[i]], send_dist[i+1]-send_dist[i]));
  }
  
  boost::mpi::wait_all(reqs.begin(), reqs.end());
}

void make_node_receive_lists(const SimpleCommunicationPattern::IndicesT& nodes_dist, CMesh& mesh, SimpleCommunicationPattern& comms_pattern)
{
	CFinfo << "making node_receive_lists" << CFendl;
  std::vector<Uint> missing_nodes;
  const Uint nodes_begin = nodes_dist[mpi::PE::instance().rank()];
  const Uint nodes_end = nodes_dist[mpi::PE::instance().rank()+1];
  
  // Find the nodes that are not stored locally
  BOOST_FOREACH(const CElements& celements, find_components_recursively<CElements>(mesh))
  {
    const CTable<Uint>::ArrayT& connectivity_table = celements.connectivity_table().array();
    BOOST_FOREACH(const CTable<Uint>::ConstRow row, connectivity_table)
    {
      BOOST_FOREACH(const Uint node_idx, row)
      {
        if(node_idx >= nodes_end || node_idx < nodes_begin)
        {
          missing_nodes.push_back(node_idx);
        }
      }
    }
  }
  
  std::sort(missing_nodes.begin(), missing_nodes.end());
  
  comms_pattern.receive_list.clear();
  comms_pattern.receive_targets.clear();
  
  // set the communications pattern
  const Uint nb_missing_nodes = missing_nodes.size();
  std::map<Uint,Uint> replace_nodes;
  for(Uint i = 0; i != nb_missing_nodes;)
  {
    const Uint node_idx = missing_nodes[i];
    const Uint node_processor = std::upper_bound(nodes_dist.begin(), nodes_dist.end(), node_idx) - 1 - nodes_dist.begin();
    
    comms_pattern.receive_targets.push_back(nodes_end - nodes_begin + comms_pattern.receive_list.size());
    comms_pattern.receive_list.push_back(node_idx - nodes_dist[node_processor]);
    comms_pattern.receive_dist[node_processor+1] = comms_pattern.receive_list.size();
    replace_nodes[node_idx] = comms_pattern.receive_targets.back();
    
    while(i != nb_missing_nodes && node_idx == missing_nodes[i])
    {
      ++i;
    }
  }
  
  // Fill missing receive dists
  for(Uint i = 1; i != comms_pattern.receive_dist.size(); ++i)
  {
    if(comms_pattern.receive_dist[i] < comms_pattern.receive_dist[i-1])
      comms_pattern.receive_dist[i] = comms_pattern.receive_dist[i-1];
  }
  
  // Update connectivity data to point to ghosts where data is missing
  const Uint nb_nodes = nodes_end - nodes_begin + comms_pattern.receive_list.size();
  BOOST_FOREACH(CElements& celements, find_components_recursively<CElements>(mesh))
  {
    CTable<Real>& coords = celements.nodes().coordinates();
		coords.resize(nb_nodes);
		CList<Uint>::Ptr global_indices = coords.get_child<CList<Uint> >("global_indices");
		if (global_indices)
			global_indices->resize(nb_nodes);
    CTable<Uint>::ArrayT& connectivity_table = celements.connectivity_table().array();
    BOOST_FOREACH(CTable<Uint>::Row row, connectivity_table)
    {
      BOOST_FOREACH(Uint& node_idx, row)
      {
        if(node_idx >= nodes_end || node_idx < nodes_begin)
        {
          node_idx = replace_nodes[node_idx];
        }
        else
        {
          node_idx -= nodes_begin;
        }
        cf_assert(node_idx < coords.size());
      }
    }
  }
	CFinfo << "finished making node_receive_lists" << CFendl;

}

std::ostream& operator<<(std::ostream& os, const SimpleCommunicationPattern& pattern)
{
	CFinfo << "outputting pattern" << CFendl;
  os << "send dist for rank " << mpi::PE::instance().rank() << ":" << "\n";
  for(Uint i = 0; i != (pattern.send_dist.size()-1); ++i)
    os << "  " << pattern.send_dist[i+1] - pattern.send_dist[i] << " to process " << i << "\n";
  
  os << "recv dist for rank " << mpi::PE::instance().rank() << ":" << "\n";
  for(Uint i = 0; i != (pattern.receive_dist.size()-1); ++i)
    os << "  " << pattern.receive_dist[i+1] - pattern.receive_dist[i] << " from process " << i << "\n";
  
  return os;
}

} // Mesh
} // CF
