// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////////////////

#include "common/PE/Comm.hpp"
#include "common/PE/CommPattern.hpp"
#include "common/Log.hpp"

#include "math/VariablesDescriptor.hpp"


#include "math/LSS/Trilinos/TrilinosDetail.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file TrilinosDetail.hpp Shared functions between Trilinos classes
  @author Tamas Banyai, Bart Janssens

**/

////////////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {
namespace LSS {

namespace detail
{

struct GidConverter
{
  GidConverter(const std::vector<Uint>& periodic_links_nodes, const std::vector<bool>& periodic_links_active, common::PE::CommPattern& cp)
  {
    cf3_assert(periodic_links_active.size() == periodic_links_nodes.size());

    const Uint nb_nodes = cp.gid()->size();
    gids.resize(nb_nodes);
    cp.gid()->pack(&gids[0]);

    common::PE::Comm& comm = common::PE::Comm::instance();
    const Uint nb_procs = comm.size();
    const int my_rank = comm.rank();

    if(periodic_links_nodes.empty())
    {
      int nb_local_nodes = 0;
      for(int i = 0; i != nb_nodes; ++i)
      {
        if(cp.isUpdatable()[i])
        {
          nb_local_nodes++;
        }
      }
      comm.all_reduce(common::PE::plus(), &nb_local_nodes, 1, &global_nb_gid);
    }
    else // When periodic links are provided, we must skip them in the GID list
    {
      cf3_assert(periodic_links_active.size() == nb_nodes);

      int nb_local_nodes = 0;
      int nb_local_periodic = 0;
      for(int i = 0; i != nb_nodes; ++i)
      {
        if(cp.isUpdatable()[i])
        {
          if(periodic_links_active[i])
            nb_local_nodes++;
          else
            nb_local_periodic++;
        }
      }

      std::vector<int> base_gid_distribution; base_gid_distribution.reserve(nb_procs);
      std::vector<int> periodic_gid_distribution; periodic_gid_distribution.reserve(nb_procs);
      if(comm.is_active())
      {
        // Get the total number of elements on each rank
        comm.all_gather(nb_local_nodes, base_gid_distribution);
        comm.all_gather(nb_local_periodic, periodic_gid_distribution);
      }
      else
      {
        base_gid_distribution.push_back(nb_local_nodes);
        periodic_gid_distribution.push_back(nb_local_periodic);

      }
      cf3_assert(base_gid_distribution.size() == nb_procs);
      cf3_assert(periodic_gid_distribution.size() == nb_procs);
      for(Uint i = 1; i != nb_procs; ++i)
      {
        base_gid_distribution[i] += base_gid_distribution[i-1];
        periodic_gid_distribution[i] += periodic_gid_distribution[i-1];
      }

      global_nb_gid = base_gid_distribution.back();

      // first gid on this rank. We renumber so all GIDs for non-periodic nodes come first
      Uint base_gid_counter = my_rank == 0 ? 0 : base_gid_distribution[my_rank-1];
      Uint periodic_gid_counter = (my_rank == 0 ? 0 : periodic_gid_distribution[my_rank-1]) + global_nb_gid;

      // Renumber gids
      for(Uint i = 0; i != nb_nodes; ++i)
      {
        if(cp.isUpdatable()[i])
        {
          gids[i] = periodic_links_active[i] ? periodic_gid_counter++ : base_gid_counter++;
        }
      }

      if(comm.is_active())
      {
        cp.insert("PeriodicGids", gids);
        cp.synchronize("PeriodicGids");
        cp.remove_component("PeriodicGids");
      }
    }
  }

  inline int operator[](const int i) const
  {
    return gids[i];
  }

  std::vector<int> gids;
  int global_nb_gid;
};

}

void create_map_data(common::PE::CommPattern& cp, const VariablesDescriptor& variables, std::vector< int >& p2m, std::vector< int >& my_global_elements, int& num_my_elements, const std::vector<Uint>& periodic_links_nodes, const std::vector<bool>& periodic_links_active)
{
  // get global ids vector
  const detail::GidConverter gid(periodic_links_nodes, periodic_links_active, cp);
  CFdebug << "Number of GIDs: " << gid.global_nb_gid << CFendl;

  num_my_elements = 0;

  const Uint nb_vars = variables.nb_vars();
  const Uint total_nb_eq = variables.size();

  const Uint nb_nodes_for_rank = cp.isUpdatable().size();
  my_global_elements.reserve(nb_nodes_for_rank*total_nb_eq);

  int nb_ghosts = 0;
  for(Uint var_idx = 0; var_idx != nb_vars; ++var_idx)
  {
    const Uint neq = variables.var_length(var_idx);
    const Uint var_offset = variables.offset(var_idx);
    const int var_start_gid = var_offset * gid.global_nb_gid;
    for (int i=0; i<nb_nodes_for_rank; i++)
    {
      if (cp.isUpdatable()[i] && !(periodic_links_active.size() && periodic_links_active[i]))
      {
        num_my_elements += neq;
        const int start_gid = var_start_gid + gid[i]*neq;
        for(int j = 0; j != neq; ++j)
        {
          my_global_elements.push_back(start_gid+j);
        }
      }
      else if (!cp.isUpdatable()[i] && !(periodic_links_active.size() && periodic_links_active[i]))
      {
        nb_ghosts += neq;
      }
    }
  }

  // process local to matrix local numbering mapper
  const int nb_local_nodes = num_my_elements / total_nb_eq;
  nb_ghosts /= total_nb_eq;
  p2m.resize(nb_nodes_for_rank*total_nb_eq);
  for(Uint var_idx = 0; var_idx != nb_vars; ++var_idx)
  {
    const Uint neq = variables.var_length(var_idx);
    const Uint var_offset = variables.offset(var_idx);
    int iupd=nb_local_nodes*var_offset;
    int ighost=num_my_elements + nb_ghosts*var_offset;
    for (int i=0; i<nb_nodes_for_rank; ++i)
    {
      const int p_start = i*total_nb_eq+var_offset;
      if (cp.isUpdatable()[i] && !(periodic_links_active.size() && periodic_links_active[i]))
      {
        for(Uint j = 0; j != neq; ++j)
          p2m[p_start + j] = iupd++;
      }
      else if (!cp.isUpdatable()[i] && !(periodic_links_active.size() && periodic_links_active[i]))
      {
        for(Uint j = 0; j != neq; ++j)
          p2m[p_start + j] = ighost++;
      }
    }
    if(!periodic_links_active.empty())
    {
      for (int i=0; i<nb_nodes_for_rank; ++i)
      {
        const int p_start = i*total_nb_eq+var_offset;
        if(periodic_links_active[i])
        {
          Uint final_linked_node = periodic_links_nodes[i];
          while(periodic_links_active[final_linked_node])
            final_linked_node = periodic_links_nodes[final_linked_node];

          const int p_linked = final_linked_node*total_nb_eq+var_offset;
          for(Uint j = 0; j != neq; ++j)
            p2m[p_start + j] = p2m[p_linked + j];

        }
      }
    }
  }

  // append the ghosts at the end of the element list
  for(Uint var_idx = 0; var_idx != nb_vars; ++var_idx)
  {
    const Uint neq = variables.var_length(var_idx);
    const Uint var_offset = variables.offset(var_idx);
    const int var_start_gid = var_offset * gid.global_nb_gid;
    for (int i=0; i<nb_nodes_for_rank; i++)
    {
      if (!cp.isUpdatable()[i] && !(periodic_links_active.size() && periodic_links_active[i]))
      {
        const int start_gid = var_start_gid + gid[i]*neq;
        for(int j = 0; j != neq; ++j)
          my_global_elements.push_back(start_gid+j);
      }
    }
  }
}


} // namespace LSS
} // namespace math
} // namespace cf3
