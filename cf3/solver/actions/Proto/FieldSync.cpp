// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/PE/Comm.hpp"
#include "common/List.hpp"

#include "FieldSync.hpp"

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

FieldSynchronizer::FieldSynchronizer()
{
}

FieldSynchronizer& FieldSynchronizer::instance()
{
  static FieldSynchronizer instance;
  return instance;
}


void FieldSynchronizer::insert(mesh::Field& f, bool do_periodic_element_update)
{
  m_fields[f.uri().path()] = std::make_pair(f.handle<mesh::Field>(), do_periodic_element_update);
}

void FieldSynchronizer::synchronize()
{
  // Periodic update needed even in a sequential run
  for(FieldsT::iterator field_it = m_fields.begin(); field_it != m_fields.end(); ++field_it)
  {
    if(!field_it->second.second)
      continue;
    
    mesh::Field& field = *field_it->second.first;
    const mesh::Dictionary& dict = field.dict();
    Handle< common::List<Uint> const > periodic_links_nodes_h(dict.get_child("periodic_links_nodes"));
    Handle< common::List<bool> const > periodic_links_active_h(dict.get_child("periodic_links_active"));
    if(is_not_null(periodic_links_nodes_h) && is_not_null(periodic_links_active_h))
    {
      const common::List<Uint>& periodic_links_nodes = *periodic_links_nodes_h;
      const common::List<bool>& periodic_links_active = *periodic_links_active_h;
      const Uint nb_nodes = periodic_links_nodes.size();
      std::vector< std::vector<Uint> > inverse_periodic_links(nb_nodes);

      for(Uint i = 0; i != nb_nodes; ++i)
      {
        if(periodic_links_active[i])
        {
          Uint final_target_node = periodic_links_nodes[i];
          while(periodic_links_active[final_target_node])
          {
            final_target_node = periodic_links_nodes[final_target_node];
          }
          inverse_periodic_links[final_target_node].push_back(i);
        }
      }

      const Uint row_size = field.row_size();

      for(Uint i = 0; i != nb_nodes; ++i)
      {
        const std::vector<Uint>& my_links = inverse_periodic_links[i];
        const Uint nb_links = my_links.size();
        if(nb_links == 0)
          continue;
        Eigen::Map<RealVector> my_row(&field[i][0], row_size);
        for(Uint j = 0; j != nb_links; ++j)
        {
          my_row += Eigen::Map<RealVector>(&field[my_links[j]][0], row_size);
        }
        for(Uint j = 0; j != nb_links; ++j)
        {
          Eigen::Map<RealVector> other_row(&field[my_links[j]][0], row_size);
          other_row = my_row;
        }
      }
    }
  }
  
  if(common::PE::Comm::instance().is_active())
  {
    for(FieldsT::iterator field_it = m_fields.begin(); field_it != m_fields.end(); ++field_it)
    {
      field_it->second.first->synchronize();
    }
  }

  m_fields.clear();
}

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3
