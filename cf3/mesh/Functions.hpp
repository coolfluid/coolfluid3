// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Functions_hpp
#define cf3_mesh_Functions_hpp

#include "common/Handle.hpp"

#include "math/MatrixTypes.hpp"

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
boost::shared_ptr< common::List< Uint > > build_used_nodes_list( const std::vector< Handle<Entities const> >& entities, const Dictionary& dictionary, const bool include_ghost_elems, const bool follow_periodic_links = true);

/// create_used_nodes_list
/// @brief Create a List<Uint> containing unique entries of all the nodes used by a vector of entities
/// @param [in]  node_user   component being entities, or holding entities somewhere down in his tree
/// @param [in]  dictionary  dictionary where the nodes are stored
/// @param [in]  include_ghost_elems  if true, ghost elements will be included in the search
/// @return used_nodes  List of used nodes
boost::shared_ptr< common::List< Uint > > build_used_nodes_list( const common::Component& node_user, const Dictionary& dictionary, const bool include_ghost_elems, const bool follow_periodic_links = true);

////////////////////////////////////////////////////////////////////////////////

/// Build a mapping linking the local source coordinates to the nearest local coordinate in support_local_coords.
/// @param node_mapping [out] Mapping from source_local_coords to indices into support_local_coords
/// @param is_interior [out] True for each source_local_coord that is an internal node, i.e. a node that is not on the element boundary.
void nearest_node_mapping(const RealMatrix& support_local_coords, const RealMatrix& source_local_coords, std::vector<Uint>& node_mapping, std::vector<bool>& is_interior);

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Functions_hpp
