// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SimpleCommunicationPattern_hpp
#define CF_Mesh_SimpleCommunicationPattern_hpp

#include <boost/foreach.hpp>

#include "Common/CF.hpp"
#include "Common/MPI/PE.hpp"

#include "Mesh/CArray.hpp"
#include "Mesh/CList.hpp"

#include "Mesh/LibMesh.hpp"

namespace CF {
namespace Mesh {

class CMesh;
  
/// Holds lists of indices to fetch from other CPUs
struct Mesh_API SimpleCommunicationPattern
{
  typedef std::vector<Uint> IndicesT;
  
  SimpleCommunicationPattern();
  
  /// Updates the send lists based on the existing receive lists
  void update_send_lists();
  
  /// Indices of items to receive from other processes
  IndicesT receive_list;
  
  /// Locations to store the received items
  IndicesT receive_targets;
  
  /// Distribution of receive items among processes, i.e. the current process wants data from
  /// receive_indices[receive_dist[i]] to receive_indices[receive_dist[i+1]] from process i
  /// Size is the number of processes + 1
  IndicesT receive_dist;
  
  /// Indices of items to send to other processes
  IndicesT send_list;
  
  /// Distribution of send list among processes
  IndicesT send_dist;
};

/// Given a mesh and the distribution of its nodes among CPUs, fill the receive lists in the communication pattern
void Mesh_API make_node_receive_lists(const SimpleCommunicationPattern::IndicesT& nodes_dist, CMesh& mesh, SimpleCommunicationPattern& comms_pattern);

/// Apply a communication pattern to the given range of CArrays.
/// RangeT must iterable by BOOST_FOREACH
template<typename RangeT>
void apply_pattern_carray(const SimpleCommunicationPattern& pattern, RangeT range)
{
	CFinfo << "applying pattern to carray" << CFendl;
  boost::mpi::communicator& world = CF::Common::PE::instance();
  const Uint nb_procs = world.size();
  
  Uint total_width = 0;
  BOOST_FOREACH(CArray& array, range)
  {
    total_width += array.row_size();
  }
  
  std::vector<Real> receive_buffer(total_width * pattern.receive_list.size());
  std::vector<Real> send_buffer;
  send_buffer.reserve(total_width * pattern.receive_list.size());
  
  // track non-blocking requests
  std::vector<boost::mpi::request> reqs;
  reqs.reserve(nb_procs*2);
  
  // Do the buffer initialization and communication
  Uint receive_begin = 0;
  for(Uint proc = 0; proc != nb_procs; ++proc)
  {
    const Uint send_begin = send_buffer.size();
    const Uint proc_begin = pattern.send_dist[proc];
    const Uint proc_end = pattern.send_dist[proc+1];
    Uint receive_size = 0;
    BOOST_FOREACH(CArray& array, range)
    {
      const Uint nb_cols = array.row_size();
      receive_size += nb_cols * (pattern.receive_dist[proc+1] - pattern.receive_dist[proc]);
      for(Uint i = proc_begin; i != proc_end; ++i)
      {
        cf_assert(pattern.send_list[i] < array.size());
        CArray::ConstRow row = array[pattern.send_list[i]];
        send_buffer.insert(send_buffer.end(), row.begin(), row.end());
      }
    }
    
		CFinfo << "proc " << proc << " sending to " << CF::Common::PE::instance().rank() << CFendl;
    // Schedule send and receive operations
    reqs.push_back(world.isend(proc, 0, &send_buffer[send_begin], send_buffer.size() - send_begin));
    reqs.push_back(world.irecv(proc, 0, &receive_buffer[receive_begin], receive_size));
    receive_begin += receive_size;
  }
  
  // Wait for the comms to be done
  boost::mpi::wait_all(reqs.begin(), reqs.end());
  
  // Unpack the receive buffer
  Uint buffer_idx = 0;
  for(Uint proc = 0; proc != nb_procs; ++proc)
  {
    const Uint proc_begin = pattern.receive_dist[proc];
    const Uint proc_end = pattern.receive_dist[proc+1];
    BOOST_FOREACH(CArray& array, range)
    {
      const Uint nb_cols = array.row_size();
      for(Uint i = proc_begin; i != proc_end; ++i)
      {
        cf_assert(pattern.receive_targets[i] < array.size());
        CArray::Row row = array[pattern.receive_targets[i]];
        std::copy(receive_buffer.begin() + buffer_idx, receive_buffer.begin() + buffer_idx + nb_cols, row.begin());
        buffer_idx += nb_cols;
      }
    }
  }
}

/// Apply a communication pattern to the given range of CArrays.
/// RangeT must iterable by BOOST_FOREACH
template<typename ValueT,typename RangeT>
void apply_pattern_clist(const SimpleCommunicationPattern& pattern, RangeT range)
{
	typedef CList<ValueT> CListT;

  boost::mpi::communicator& world = CF::Common::PE::instance();
  const Uint nb_procs = world.size();
  
	Uint total_width = 0;
  BOOST_FOREACH(CList<ValueT>& list, range)
  {
    ++total_width;
  }
	
  std::vector<ValueT> receive_buffer(total_width * pattern.receive_list.size());
  std::vector<ValueT> send_buffer;
  send_buffer.reserve(total_width * pattern.receive_list.size());
  
  // track non-blocking requests
  std::vector<boost::mpi::request> reqs;
  reqs.reserve(nb_procs*2);
  
  // Do the buffer initialization and communication
  Uint receive_begin = 0;
  for(Uint proc = 0; proc != nb_procs; ++proc)
  {
    const Uint send_begin = send_buffer.size();
    const Uint proc_begin = pattern.send_dist[proc];
    const Uint proc_end = pattern.send_dist[proc+1];
    Uint receive_size = 0;
    BOOST_FOREACH(CListT& array, range)
    {
      const Uint nb_cols = 1;
      receive_size += nb_cols * (pattern.receive_dist[proc+1] - pattern.receive_dist[proc]);
      for(Uint i = proc_begin; i != proc_end; ++i)
      {
        cf_assert(pattern.send_list[i] < array.size());
        ValueT& row = array[pattern.send_list[i]];
        send_buffer.insert(send_buffer.end(), row);
      }
    }
    
    // Schedule send and receive operations
    reqs.push_back(world.isend(proc, 0, &send_buffer[send_begin], send_buffer.size() - send_begin));
    reqs.push_back(world.irecv(proc, 0, &receive_buffer[receive_begin], receive_size));
    receive_begin += receive_size;
  }
  
  // Wait for the comms to be done
  boost::mpi::wait_all(reqs.begin(), reqs.end());
  
  // Unpack the receive buffer
  Uint buffer_idx = 0;
  for(Uint proc = 0; proc != nb_procs; ++proc)
  {
    const Uint proc_begin = pattern.receive_dist[proc];
    const Uint proc_end = pattern.receive_dist[proc+1];
    BOOST_FOREACH(CListT& array, range)
    {
      const Uint nb_cols = 1;
      for(Uint i = proc_begin; i != proc_end; ++i)
      {
        cf_assert(pattern.receive_targets[i] < array.size());
        ValueT& row = array[pattern.receive_targets[i]];
        std::copy(receive_buffer.begin() + buffer_idx, receive_buffer.begin() + buffer_idx + nb_cols, &row);
        buffer_idx += nb_cols;
      }
    }
  }
}

// Stream output
Mesh_API std::ostream& operator<<(std::ostream& os, const SimpleCommunicationPattern& pattern);

} // Mesh
} // CF

#endif /* CF_Mesh_SimpleCommunicationPattern_hpp */
