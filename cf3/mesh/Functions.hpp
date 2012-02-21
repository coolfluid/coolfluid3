// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Functions_hpp
#define cf3_mesh_Functions_hpp

#include "common/Handle.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common { template <typename T> class List;}
namespace mesh {

  class Dictionary;
  class Entities;

////////////////////////////////////////////////////////////////////////////////

/// create_used_nodes_list
/// @brief Create a List<Uint> containing unique entries of all the nodes used by a vector of entities
/// @param [in]  entities    vector of entities, that the unique nodes will be collected from
/// @param [in]  dictionary  dictionary where the nodes are stored
/// @return used_nodes  List of used nodes
boost::shared_ptr< common::List< Uint > > create_used_nodes_list( const std::vector< Handle<Entities const> >& entities, const Dictionary& dictionary);

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Functions_hpp
