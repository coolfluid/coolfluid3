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

void create_map_data(common::PE::CommPattern& cp, const VariablesDescriptor& variables, std::vector< int >& p2m, std::vector< int >& my_global_elements, int& num_my_elements)
{
  // get global ids vector
  int *gid=(int*)cp.gid()->pack();
  num_my_elements = 0;

  const Uint nb_vars = variables.nb_vars();
  const Uint total_nb_eq = variables.size();

  const Uint nb_nodes_for_rank = cp.isUpdatable().size();
  my_global_elements.reserve(nb_nodes_for_rank*total_nb_eq);

  // Get the maximum gid, for per-equation blocked storage
  int local_max_gid = 0;
  int global_nb_gid = 0;
  for(Uint i = 0; i != nb_nodes_for_rank; ++i)
    local_max_gid = gid[i] > local_max_gid ? gid[i] : local_max_gid;

  common::PE::Comm::instance().all_reduce(common::PE::max(), &local_max_gid, 1, &global_nb_gid);
  ++global_nb_gid; // number of GIDs is the maximum + 1
  CFdebug << "Number of GIDs: " << global_nb_gid << CFendl;

  for(Uint var_idx = 0; var_idx != nb_vars; ++var_idx)
  {
    const Uint neq = variables.var_length(var_idx);
    const Uint var_offset = variables.offset(var_idx);
    const int var_start_gid = var_offset * global_nb_gid;
    for (int i=0; i<nb_nodes_for_rank; i++)
    {
      if (cp.isUpdatable()[i])
      {
        num_my_elements += neq;
        const int start_gid = var_start_gid + gid[i]*neq;
        for(int j = 0; j != neq; ++j)
        {
          my_global_elements.push_back(start_gid+j);
        }
      }
    }
  }

  // process local to matrix local numbering mapper
  const int nb_local_nodes = num_my_elements / total_nb_eq;
  const int nb_ghosts = nb_nodes_for_rank - nb_local_nodes;
  p2m.resize(nb_nodes_for_rank*total_nb_eq);
  for(Uint var_idx = 0; var_idx != nb_vars; ++var_idx)
  {
    const Uint neq = variables.var_length(var_idx);
    const Uint var_offset = variables.offset(var_idx);
    int iupd=nb_local_nodes*var_offset;
    int ighost=num_my_elements + nb_ghosts*var_offset;
    int p_idx = 0;
    for (int i=0; i<nb_nodes_for_rank; ++i)
    {
      const int p_start = i*total_nb_eq+var_offset;
      if (cp.isUpdatable()[i])
      {
        for(Uint j = 0; j != neq; ++j)
          p2m[p_start + j] = iupd++;
      }
      else
      {
        for(Uint j = 0; j != neq; ++j)
          p2m[p_start + j] = ighost++;
      }
    }
  }

  // append the ghosts at the end of the element list
  for(Uint var_idx = 0; var_idx != nb_vars; ++var_idx)
  {
    const Uint neq = variables.var_length(var_idx);
    const Uint var_offset = variables.offset(var_idx);
    const int var_start_gid = var_offset * global_nb_gid;
    for (int i=0; i<nb_nodes_for_rank; i++)
    {
      if (!cp.isUpdatable()[i])
      {
        const int start_gid = var_start_gid + gid[i]*neq;
        for(int j = 0; j != neq; ++j)
          my_global_elements.push_back(start_gid+j);
      }
    }
  }

  delete[] gid;
}


} // namespace LSS
} // namespace math
} // namespace cf3
