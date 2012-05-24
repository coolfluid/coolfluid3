// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_SparsityBuilder_hpp
#define cf3_UFEM_SparsityBuilder_hpp

#include "UFEM/LibUFEM.hpp"

namespace cf3 {
  namespace common {
    template<class T >
    class List;
  }

  namespace mesh {
    class Region;
    class Dictionary;
  }
namespace UFEM {

////////////////////////////////////////////////////////////////////////////////////////////

/// Build the sparsity structure for a LSS. This function assumes that the solution
/// is in the same space as the geometry.
/// @param mesh Mesh on which the solver is run. All Cells below mesh.topology() are considered
/// @param node_connectivity Lists the connected nodes for each node.
/// @param start_indices For each node N, the index in node_connectivity where the list of connected nodes of node N starts.
/// Size is number of nodes + 1, so the last item is the size of node_connectivity
UFEM_API boost::shared_ptr< common::List< Uint > > build_sparsity(const std::vector< Handle<mesh::Region> >& regions, const mesh::Dictionary& dictionary, std::vector<Uint>& node_connectivity, std::vector<Uint>& start_indices, common::List<Uint>& gids, common::List<Uint>& ranks, common::List<Uint>& used_node_map);

////////////////////////////////////////////////////////////////////////////////////////////

} // UFEM
} // cf3

#endif // cf3_UFEM_SparsityBuilder_hpp
