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

#include "Solver/Actions/CNodeOperation.hpp"
#include "Solver/Actions/CForAllNodes.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace Solver {
namespace Actions {
  
ComponentBuilder < CForAllNodes, CLoop, LibActions > CForAllNodes_builder;

/////////////////////////////////////////////////////////////////////////////////////

CForAllNodes::CForAllNodes ( const std::string& name ) :
  CLoop(name)
{
}
  
void CForAllNodes::execute()
{
  boost_foreach(CRegion::Ptr& region, m_loop_regions)
  {
    // CNodes& nodes = find_parent_component<CMesh>(*region).nodes();
    // CList<Uint>::Ptr used_nodes = region->find_component_ptr_with_tag<CList<Uint> >(*region,"used_nodes");
    // if (m_update_nodes || is_null(used_nodes))
    // {
    //   if (is_null(used_nodes))
    //   {
    //     used_nodes = region->allocate_component<CList<Uint> >("used_nodes");
    //     used_nodes->add_tag("used_nodes");
    //   }
    //   // Assemble all unique node numbers in a set
    //   std::set<Uint> node_set;
    //   boost_foreach(CElements& elements, find_components_recursively<CElements>(*region))
    //   {
    //     boost_foreach(CTable<Uint>::ConstRow elem_nodes, elements.connectivity_table().array())
    //     {
    //       boost_foreach(const Uint node, elem_nodes)
    //       {
    //         node_set.insert(node);
    //       }
    //     }
    //   }
    //   // Copy the set to the node_list
    //   used_nodes->resize(node_set.size());
    //   CList<Uint>::Array& nodes_array = used_nodes->array();
    //   index_foreach(i,const Uint node, node_set)
    //     nodes_array[i] = node;
    // }
    
    boost_foreach(CElements& elements, find_components_recursively<CElements>(*region))
    {
      // Setup all child operations
      CList<Uint>::Ptr loop_list;
      boost_foreach(CNodeOperation& op, find_components<CNodeOperation>(*this))
      {
        op.create_loop_helper( elements );

        if ( is_null(loop_list) )
          loop_list = op.loop_list().as_type< CList<Uint> >();
        else if (loop_list->size() != op.loop_list().size())
          throw BadValue(FromHere(), "The number of nodes of CNodeOperation [" + op.name() + "] doesn't match with other operations in the same loop");
      }
      boost_foreach(const Uint node, loop_list->array())
      {
        boost_foreach(CNodeOperation& op, find_components<CNodeOperation>(*this))
        {
          op.select_loop_idx(node);
          op.execute();
        }
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

/////////////////////////////////////////////////////////////////////////////////////
