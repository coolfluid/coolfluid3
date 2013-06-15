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

boost::shared_ptr< List< Uint > > build_used_nodes_list( const std::vector< Handle<Entities const> >& entities_vector, const Dictionary& dictionary, bool include_ghost_elems, const bool follow_periodic_links)
{
  boost::shared_ptr< List< Uint > > used_nodes = allocate_component< List< Uint > >(mesh::Tags::nodes_used());

  if(entities_vector.empty())
    return used_nodes;

  const Uint all_nb_nodes = dictionary.size();

  std::vector<bool> node_is_used(all_nb_nodes, false);
  const List<Uint>* periodic_links_nodes = dynamic_cast< const List<Uint>* >(dictionary.get_child("periodic_links_nodes").get());
  const List<bool>* periodic_links_active = dynamic_cast< const List<bool>* >(dictionary.get_child("periodic_links_active").get());

  // First count the number of unique nodes
  Uint nb_nodes = 0;
  boost_foreach(const Handle<Entities const>& entities, entities_vector)
  {
    const Space& space = entities->space(dictionary);
    const Uint nb_elems = space.size();

    for (Uint idx=0; idx<nb_elems; ++idx)
    {
      // Don't include ghost-elements if not requested
      if(include_ghost_elems || entities->is_ghost(idx) == false)
      {
        boost_foreach(const Uint node, space.connectivity()[idx])
        {
          cf3_assert(node<node_is_used.size());
          if(!node_is_used[node])
          {
            node_is_used[node] = true;
            ++nb_nodes;
          }
          if(follow_periodic_links && periodic_links_active)
          {
            Uint tgt_node = node;
            while((*periodic_links_active)[tgt_node])
            {
              tgt_node = (*periodic_links_nodes)[tgt_node];
              if(!node_is_used[tgt_node])
              {
                node_is_used[tgt_node] = true;
                ++nb_nodes;
              }
            }
          }
        }
      }
    }
  }

  // reserve space for all unique nodes
  used_nodes->resize(nb_nodes);
  common::List<Uint>::ListT& nodes_array = used_nodes->array();

  // Add the unique node indices
  node_is_used.assign(all_nb_nodes, false);
  Uint back = 0;
  boost_foreach(const Handle<Entities const>& entities, entities_vector)
  {
    const Space& space = entities->space(dictionary);
    const Uint nb_elems = space.size();
    for (Uint idx=0; idx<nb_elems; ++idx)
    {
      // Don't include ghost-elements if not requested
      if(include_ghost_elems || entities->is_ghost(idx) == false)
      {
        boost_foreach(const Uint node, space.connectivity()[idx])
        {
          if(!node_is_used[node])
          {
            node_is_used[node] = true;
            nodes_array[back++] = node;
          }
          if(follow_periodic_links && periodic_links_active)
          {
            Uint tgt_node = node;
            while((*periodic_links_active)[tgt_node])
            {
              tgt_node = (*periodic_links_nodes)[tgt_node];
              if(!node_is_used[tgt_node])
              {
                node_is_used[tgt_node] = true;
                nodes_array[back++] = tgt_node;
              }
            }
          }
        }
      }
    }
  }

  std::sort(used_nodes->array().begin(), used_nodes->array().end());
  return used_nodes;
}

////////////////////////////////////////////////////////////////////////////////

boost::shared_ptr< common::List< Uint > > build_used_nodes_list( const Component& node_user, const Dictionary& dictionary, bool include_ghost_elems, const bool follow_periodic_links)
{
  std::vector< Handle<Entities const> > entities_vector;
  if (Handle<Entities const> entities_h = node_user.handle<Entities>())
    entities_vector.push_back(entities_h);
  else
    entities_vector = range_to_const_vector( find_components_recursively<Entities>(node_user) );

  return build_used_nodes_list(entities_vector,dictionary,include_ghost_elems, follow_periodic_links);
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
