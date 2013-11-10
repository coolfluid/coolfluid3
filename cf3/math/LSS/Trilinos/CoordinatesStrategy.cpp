// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////////////////

#include "CoordinatesStrategy.hpp"

namespace cf3 {
namespace math {
namespace LSS {


CoordinatesStrategy::CoordinatesStrategy(const std::string& name) :
  SolutionStrategy(name)
{
}

CoordinatesStrategy::~CoordinatesStrategy()
{
}

void CoordinatesStrategy::set_coordinates(common::PE::CommPattern& cp, const common::Table< Real >& coords, const common::List< Uint >& used_nodes, const std::vector< bool >& periodic_links_active)
{
  const Uint nb_nodes = used_nodes.size();
  m_x_coords.reserve(nb_nodes);
  m_y_coords.reserve(nb_nodes);
  m_z_coords.reserve(nb_nodes);
  const Uint dim = coords.row_size();
  cf3_assert(dim != 0);
  
  // First add the updatable nodes
  for(Uint i = 0; i != nb_nodes; ++i)
  {
    if(cp.isUpdatable()[i] && !(periodic_links_active.size() && periodic_links_active[i]))
    {
      const Uint node_idx = used_nodes[i];
      m_x_coords.push_back(coords[node_idx][0]);
      if(dim > 1)
        m_y_coords.push_back(coords[node_idx][1]);
      if(dim > 2)
        m_z_coords.push_back(coords[node_idx][2]);
    }
  }
  
  // Then put the ghosts in the end
  for(Uint i = 0; i != nb_nodes; ++i)
  {
    if(!cp.isUpdatable()[i] && !(periodic_links_active.size() && periodic_links_active[i]))
    {
      const Uint node_idx = used_nodes[i];
      m_x_coords.push_back(coords[node_idx][0]);
      if(dim > 1)
        m_y_coords.push_back(coords[node_idx][1]);
      if(dim > 2)
        m_z_coords.push_back(coords[node_idx][2]);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

} // namespace LSS
} // namespace math
} // namespace cf3
