// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UFEM_SparsityBuilder_hpp
#define CF_UFEM_SparsityBuilder_hpp

#include "UFEM/LibUFEM.hpp"

namespace CF {
  namespace Mesh { class CMesh; }
namespace UFEM {

////////////////////////////////////////////////////////////////////////////////////////////

/// Build the sparsity structure for a LSS. This function assumes that the solution
/// is in the same space as the geometry.
/// @param mesh Mesh on which the solver is run. All CCells below mesh.topology() are considered
/// @param node_connectivity Lists the connected nodes for each node.
/// @param start_indices For each node N, the index in node_connectivity where the list of connected nodes of node N starts.
/// Size is number of nodes + 1, so the last item is the size of node_connectivity
void UFEM_API build_sparsity(const Mesh::CMesh& mesh, std::vector<Uint>& node_connectivity, std::vector<Uint>& start_indices);

////////////////////////////////////////////////////////////////////////////////////////////

} // UFEM
} // CF

#endif // CF_UFEM_SparsityBuilder_hpp
