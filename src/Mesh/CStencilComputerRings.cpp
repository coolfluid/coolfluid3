// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include <boost/tuple/tuple.hpp>

#include "Common/Foreach.hpp"
#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/FindComponents.hpp"
#include "Common/OptionT.hpp"

#include "Mesh/CStencilComputerRings.hpp"
#include "Mesh/CNodeElementConnectivity.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/ElementType.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/COcttree.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

  using namespace Common;
  
////////////////////////////////////////////////////////////////////////////////

CF::Common::ComponentBuilder < CStencilComputerRings, CStencilComputer, LibMesh > CStencilComputerRings_Builder;

//////////////////////////////////////////////////////////////////////////////

CStencilComputerRings::CStencilComputerRings( const std::string& name )
  : CStencilComputer(name), m_nb_rings(0)
{  
  property("mesh").as_option().attach_trigger(boost::bind(&CStencilComputerRings::configure_mesh,this));
    
  properties().add_option(OptionT<Uint>::create("nb_rings","Number of Rings", "Number of neighboring rings of elements in stencil", m_nb_rings))
    ->link_to(&m_nb_rings);

}

//////////////////////////////////////////////////////////////////////

void CStencilComputerRings::configure_mesh()
{
  if (m_mesh.expired())
    throw SetupError(FromHere(), "Option \"mesh\" has not been configured");
    
  CMesh& mesh = *m_mesh.lock();
  CNodeElementConnectivity::Ptr node2cell_ptr = find_component_ptr<CNodeElementConnectivity>(mesh);
  if (is_null(node2cell_ptr))
  {
    node2cell_ptr = mesh.create_component<CNodeElementConnectivity>("node_to_cell");
    node2cell_ptr->add_elements(unified_elements().data_components());
    node2cell_ptr->build_connectivity();
  }
  m_node2cell = node2cell_ptr;
}

//////////////////////////////////////////////////////////////////////////////

void CStencilComputerRings::compute_stencil(const Uint unified_elem_idx, std::vector<Uint>& stencil)
{
  std::set<Uint> included;
  visited_nodes.clear();
  compute_neighbors(included,unified_elem_idx);
  
  if (included.size() < m_min_stencil_size)
    CFwarn << "stencil size computed for element " << unified_elem_idx << " is " << included.size() <<". This is smaller than the requested " << m_min_stencil_size << "." << CFendl;
  
  stencil.clear(); stencil.reserve(included.size());
  boost_foreach (const Uint elem, included)
    stencil.push_back(elem);
}

////////////////////////////////////////////////////////////////////////////////

void CStencilComputerRings::compute_neighbors(std::set<Uint>& included, const Uint unified_elem_idx, const Uint level)
{ 
  included.insert(unified_elem_idx);
  
  if (level < m_nb_rings)
  {
    CElements::Ptr elements;
    Uint elem_idx;
    std::set<Uint>::iterator it;
    bool inserted;
    boost::tie(elements,elem_idx) = unified_elements().data_location(unified_elem_idx);
    boost_foreach(Uint node_idx, elements->connectivity_table()[elem_idx])
    {
      boost_foreach(Uint neighbor_elem, node2cell().elements(node_idx))
      {
        compute_neighbors(included,neighbor_elem,level+1);
      }        
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
