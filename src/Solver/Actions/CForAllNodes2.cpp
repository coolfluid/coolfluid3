// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/Foreach.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/CList.hpp"
#include "Mesh/CMesh.hpp"

#include "Solver/Actions/CNodeOperation.hpp"
#include "Solver/Actions/CForAllNodes2.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace Solver {
namespace Actions {
  
ComponentBuilder < CForAllNodes2, CLoop, LibActions > CForAllNodes2_builder;

/////////////////////////////////////////////////////////////////////////////////////

CForAllNodes2::CForAllNodes2 ( const std::string& name ) :
  CLoop(name),
  m_update_nodes(false)
{
}
  
void CForAllNodes2::execute()
{
  boost_foreach(CRegion::Ptr& region, m_loop_regions)
  {
    CNodes& nodes = find_parent_component<CMesh>(*region).nodes();
    CList<Uint>::Ptr used_nodes = find_component_ptr_with_tag<CList<Uint> >(*region,"used_nodes");
    if (m_update_nodes || is_null(used_nodes))
    {
      if (is_null(used_nodes))
      {
        used_nodes = region->create_component<CList<Uint> >("used_nodes");
        used_nodes->add_tag("used_nodes");
      }
      // Assemble all unique node numbers in a set
      std::set<Uint> node_set;
      boost_foreach(CElements& elements, find_components_recursively<CElements>(*region))
      {
        boost_foreach(CTable<Uint>::ConstRow elem_nodes, elements.connectivity_table().array())
        {
          boost_foreach(const Uint node, elem_nodes)
          {
            node_set.insert(node);
          }
        }
      }
      // Copy the set to the node_list
      used_nodes->resize(node_set.size());
      CList<Uint>::ListT& nodes_array = used_nodes->array();
      index_foreach(i,const Uint node, node_set)
        nodes_array[i] = node;
    }
    boost_foreach(const Uint node, used_nodes->array())
    {
      boost_foreach(CNodeOperation& op, find_components<CNodeOperation>(*this))
      {
        op.select_loop_idx(node);
        op.execute();
      }
    }       
  }
}

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

/////////////////////////////////////////////////////////////////////////////////////
