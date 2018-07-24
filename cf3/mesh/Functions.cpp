// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/FindComponents.hpp"
#include "common/List.hpp"

#include "mesh/Connectivity.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Space.hpp"
#include "mesh/Entities.hpp"

namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////

using namespace common;

////////////////////////////////////////////////////////////////////////////////

boost::shared_ptr< List< Uint > > build_used_nodes_list( const std::vector< Handle<Entities const> >& entities_vector, const Dictionary& dictionary, const bool include_ghost_elems, const bool follow_periodic_links)
{
  boost::shared_ptr< List< Uint > > used_nodes = allocate_component< List< Uint > >(mesh::Tags::nodes_used());

  if(entities_vector.empty())
    return used_nodes;

  const Uint all_nb_nodes = dictionary.size();

  std::vector<bool> node_is_used(all_nb_nodes, false);
  const List<Uint>* periodic_links_nodes = dynamic_cast< const List<Uint>* >(dictionary.get_child("periodic_links_nodes").get());
  const List<bool>* periodic_links_active = dynamic_cast< const List<bool>* >(dictionary.get_child("periodic_links_active").get());

  // First count the number of unique nodes
  boost_foreach( const Handle<Space>& space, dictionary.spaces() )
  {
    boost_foreach( const Handle<Entities const>& entities, entities_vector )
    {
      if( &space->support() == entities.get() )
      {
        const Uint nb_elems = space->size();

        for (Uint idx=0; idx<nb_elems; ++idx)
        {
          // Don't include ghost-elements if not requested
          if(include_ghost_elems || entities->is_ghost(idx) == false)
          {
            boost_foreach(const Uint node, space->connectivity()[idx])
            {
              cf3_assert(node<node_is_used.size());
              if(!node_is_used[node])
              {
                node_is_used[node] = true;
              }
            }
          }
        }
      }
    }
  }

  if(follow_periodic_links && periodic_links_active)
  {
    const List<Uint>& per_links = *periodic_links_nodes;
    const List<bool>& per_active = *periodic_links_active;
    // Any node connected periodically to a used node is also used
    for(Uint i = 0; i != all_nb_nodes; ++i)
    {
      if(per_active[i] && (node_is_used[per_links[i]] || node_is_used[i]))
      {
        node_is_used[i] = true;
        node_is_used[per_links[i]] = true;
      }
    }
  }

  // reserve space for all unique nodes
  const Uint nb_used_nodes = std::count(node_is_used.begin(), node_is_used.end(), true);
  used_nodes->resize(nb_used_nodes);
  common::List<Uint>::ListT& nodes_array = used_nodes->array();
  

  // Add the unique node indices
  Uint back = 0;
  for(Uint i = 0; i != all_nb_nodes; ++i)
  {
    if(node_is_used[i])
    {
      cf3_assert(back < nb_used_nodes);
      nodes_array[back++] = i;
    }
  }

  std::sort(used_nodes->array().begin(), used_nodes->array().end());
  return used_nodes;
}

////////////////////////////////////////////////////////////////////////////////

boost::shared_ptr< common::List< Uint > > build_used_nodes_list( const Component& node_user, const Dictionary& dictionary, const bool include_ghost_elems, const bool follow_periodic_links)
{
  std::vector< Handle<Entities const> > entities_vector;
  if (Handle<Entities const> entities_h = node_user.handle<Entities>())
    entities_vector.push_back(entities_h);
  else
    entities_vector = range_to_const_vector( find_components_recursively<Entities>(node_user) );

  return build_used_nodes_list(entities_vector,dictionary,include_ghost_elems, follow_periodic_links);
}

////////////////////////////////////////////////////////////////////////////////

void nearest_node_mapping(const RealMatrix& support_local_coords, const RealMatrix& source_local_coords, std::vector<Uint>& node_mapping, std::vector<bool>& is_interior)
{
  const Real eps = 1e-8;
  const Real min = support_local_coords.minCoeff();
  const Real max = support_local_coords.maxCoeff();
  const Real max_sum = min == -1. ? -2. : 1.-eps;

  const Eigen::Array<bool, Eigen::Dynamic, 1> is_interior_arr = ((source_local_coords.array() > min).rowwise().all() && (source_local_coords.array() < max).rowwise().all() && source_local_coords.array().rowwise().sum() < max_sum);

  const Uint nb_target_nodes = source_local_coords.rows();

  node_mapping.resize(nb_target_nodes);
  is_interior.resize(nb_target_nodes);

  Eigen::Array<int, Eigen::Dynamic, 1> counts(support_local_coords.rows());
  counts.setZero();

  for(Uint row_idx = 0; row_idx != nb_target_nodes; ++row_idx)
  {
    is_interior[row_idx] = is_interior_arr[row_idx];

    // Compute the distances between the target node and the support nodes
    const Eigen::Array<Real, Eigen::Dynamic, 1> distances = (support_local_coords.rowwise() - source_local_coords.row(row_idx)).rowwise().squaredNorm();

    // Minimal distance
    const Real min_dist = distances.minCoeff();

    // Index of the nearest node that has been selected the fewest times
    int min_idx;
    (distances.array() == min_dist).select(counts, nb_target_nodes).minCoeff(&min_idx);

    node_mapping[row_idx] = min_idx;
    ++counts[min_idx];
  }
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
