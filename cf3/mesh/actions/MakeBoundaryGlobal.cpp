// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"

#include "common/PE/Comm.hpp"
#include "common/PE/debug.hpp"

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

#include "MakeBoundaryGlobal.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < MakeBoundaryGlobal, MeshTransformer, mesh::actions::LibActions> MakeBoundaryGlobal_Builder;

////////////////////////////////////////////////////////////////////////////////

MakeBoundaryGlobal::MakeBoundaryGlobal(const std::string& name) : MeshTransformer(name)
{
}

void MakeBoundaryGlobal::execute()
{
  common::PE::Comm& comm = common::PE::Comm::instance();
  const Uint nb_procs = comm.size();

  if(nb_procs == 1)
    return; // Boundary is always global if we only have one process

  cf3_assert(comm.is_active());

  Mesh& mesh = *m_mesh;
  common::List<Uint>& node_gids = mesh.geometry_fields().glb_idx();
  const std::set<Uint> my_gid_set(node_gids.array().begin(), node_gids.array().end());

  MeshAdaptor adaptor(mesh);
  adaptor.prepare();

  std::vector< std::vector< std::vector< Uint > > > elements_to_send (PE::Comm::instance().size(),
                                                                              std::vector< std::vector<Uint> > (m_mesh->elements().size()));

  std::vector<Uint> new_gids, new_ranks;
  boost_foreach(mesh::Elements& elements, common::find_components_recursively_with_filter<mesh::Elements>(mesh.topology(), IsElementsSurface()))
  {
    boost::shared_ptr< common::List<Uint> > used_nodes_ptr = build_used_nodes_list(elements, mesh.geometry_fields(), false);
    common::List<Uint>& used_nodes = *used_nodes_ptr;
    const Uint nb_used_nodes = used_nodes.size();
    std::vector<Uint> used_gids(nb_used_nodes);
    for(Uint i = 0; i != nb_used_nodes; ++i)
    {
      used_gids[i] = node_gids[used_nodes[i]];
    }

    std::vector< std::vector<Uint> > received_used_gids;
    comm.all_gather(used_gids, received_used_gids);
    Uint total_nb_gids = 0;
    for(Uint rank = 0; rank != nb_procs; ++rank)
    {
      total_nb_gids += received_used_gids[rank].size();
    }
    new_gids.reserve(new_gids.size() + total_nb_gids);
    new_ranks.reserve(new_ranks.size() + total_nb_gids);

    for(Uint rank = 0; rank != nb_procs; ++rank)
    {
      if(rank == comm.rank())
        continue;

      BOOST_FOREACH(const Uint gid, received_used_gids[rank])
      {
        if(!my_gid_set.count(gid))
        {
          new_gids.push_back(gid);
          new_ranks.push_back(rank);
        }
      }
    }
  }
  const Uint old_node_count = node_gids.size();
  mesh.geometry_fields().resize(old_node_count + new_gids.size());
  common::List<Uint>& node_ranks = mesh.geometry_fields().rank();
  std::copy(new_gids.begin(), new_gids.end(), node_gids.array().begin() + old_node_count);
  std::copy(new_ranks.begin(), new_ranks.end(), node_ranks.array().begin() + old_node_count);
  mesh.update_structures();
  mesh.geometry_fields().coordinates().parallelize();
  mesh.geometry_fields().coordinates().synchronize();
}

//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
