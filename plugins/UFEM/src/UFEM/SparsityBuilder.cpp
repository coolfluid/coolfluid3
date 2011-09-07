// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "Common/FindComponents.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/Geometry.hpp"

#include "UFEM/SparsityBuilder.hpp"

namespace CF {
namespace UFEM {

using namespace Common;
using namespace Mesh;

////////////////////////////////////////////////////////////////////////////////

void build_sparsity(const CMesh& mesh, std::vector< Uint >& node_connectivity, std::vector< Uint >& start_indices)
{
  const Uint nb_nodes = mesh.geometry().coordinates().size();
  std::vector< std::set<Uint> > connectivity_sets(nb_nodes);
  start_indices.assign(nb_nodes+1, 0);

  // Determine the number of connected nodes for each element
  BOOST_FOREACH(const CElements& elements, find_components_recursively<CCells>(mesh.topology()))
  {
    const CConnectivity& connectivity = elements.node_connectivity();
    const Uint nb_elems = connectivity.size();
    const Uint nb_elem_nodes = connectivity.row_size();
    for(Uint elem = 0; elem != nb_elems; ++elem)
    {
      BOOST_FOREACH(const Uint node_a, connectivity[elem])
      {
        BOOST_FOREACH(const Uint node_b, connectivity[elem])
        {
          connectivity_sets[node_a].insert(node_b);
        }
      }
    }
  }

  // Sum the number of connected nodes to get the real start indices
  const Uint start_indices_end = start_indices.size();
  for(Uint i = 1; i != start_indices_end; ++i)
  {
    start_indices[i] = connectivity_sets[i-1].size() + start_indices[i-1];
  }

  node_connectivity.resize(start_indices.back());
  for(Uint node = 0; node != nb_nodes; ++node)
  {
    node_connectivity.insert(node_connectivity.begin() + start_indices[node], connectivity_sets[node].begin(), connectivity_sets[node].end());
  }
}


////////////////////////////////////////////////////////////////////////////////

} // UFEM
} // CF
