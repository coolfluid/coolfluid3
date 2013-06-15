// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Functions_hpp
#define cf3_mesh_Functions_hpp

#include "common/Handle.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
class Component;
template <typename T> class List;
}
namespace mesh {

  class Dictionary;
  class Entities;

////////////////////////////////////////////////////////////////////////////////

/// create_used_nodes_list
/// @brief Create a List<Uint> containing unique entries of all the nodes used by a vector of entities
/// @param [in]  entities             vector of entities, that the unique nodes will be collected from
/// @param [in]  dictionary           dictionary where the nodes are stored
/// @param [in]  include_ghost_elems  if true, ghost elements will be included in the search
/// @return used_nodes  List of used nodes
boost::shared_ptr< common::List< Uint > > build_used_nodes_list( const std::vector< Handle<Entities const> >& entities, const Dictionary& dictionary, bool include_ghost_elems, const bool follow_periodic_links = true);

/// create_used_nodes_list
/// @brief Create a List<Uint> containing unique entries of all the nodes used by a vector of entities
/// @param [in]  node_user   component being entities, or holding entities somewhere down in his tree
/// @param [in]  dictionary  dictionary where the nodes are stored
/// @param [in]  include_ghost_elems  if true, ghost elements will be included in the search
/// @return used_nodes  List of used nodes
boost::shared_ptr< common::List< Uint > > build_used_nodes_list( const common::Component& node_user, const Dictionary& dictionary, bool include_ghost_elems, const bool follow_periodic_links = true);

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Functions_hpp
