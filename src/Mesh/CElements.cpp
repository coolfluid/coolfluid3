// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "Common/Log.hpp"
#include "Common/CLink.hpp"
#include "Common/CGroup.hpp"
#include "Common/CreateComponent.hpp"

#include "Mesh/CElements.hpp"
#include "Mesh/CConnectivity.hpp"
#include "Mesh/CList.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CSpace.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CElements, CEntities, LibMesh > CElements_Builder;

////////////////////////////////////////////////////////////////////////////////

CElements::CElements ( const std::string& name ) :
  CEntities ( name )
{
  properties()["brief"] = std::string("Holds information of elements of one type");
  properties()["description"] = std::string("Container component that stores the element to node connectivity,\n")
  +std::string("a link to node storage, a list of used nodes, and global numbering unique over all processors");
  
  m_node_connectivity = create_static_component<CConnectivity>(Mesh::Tags::connectivity_table());
  m_node_connectivity->add_tag(Mesh::Tags::connectivity_table());
  m_node_connectivity->properties()["brief"] = std::string("The connectivity table specifying for each element the nodes in the coordinates table");
  
}

////////////////////////////////////////////////////////////////////////////////

CElements::~CElements()
{
}

//////////////////////////////////////////////////////////////////////////////

void CElements::initialize(const std::string& element_type_name, CNodes& nodes)
{
  CEntities::initialize(element_type_name,nodes);
  node_connectivity().set_row_size(m_element_type->nb_nodes());
  node_connectivity().create_lookup().add(nodes);
  create_space0();
}

//////////////////////////////////////////////////////////////////////////////

CConnectivity& CElements::node_connectivity()
{
  return *m_node_connectivity;
}

//////////////////////////////////////////////////////////////////////////////

const CConnectivity& CElements::node_connectivity() const
{
  return *m_node_connectivity;
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix CElements::get_coordinates(const Uint elem_idx) const
{
  const CTable<Real>& coords_table = nodes().coordinates();
  CConnectivity::ConstRow elem_nodes = node_connectivity()[elem_idx];

  const Uint nb_nodes=elem_nodes.size();
  const Uint dim=coords_table.row_size();
  RealMatrix elem_coords(nb_nodes,dim);
  
  for(Uint node = 0; node != nb_nodes; ++node)
    for (Uint d=0; d<dim; ++d)
      elem_coords(node,d) = coords_table[elem_nodes[node]][d];

  return elem_coords;
}

////////////////////////////////////////////////////////////////////////////////

void CElements::put_coordinates(RealMatrix& elem_coords, const Uint elem_idx) const
{
  const CTable<Real>& coords_table = nodes().coordinates();
  CConnectivity::ConstRow elem_nodes = node_connectivity()[elem_idx];

  const Uint nb_nodes=elem_coords.rows();
  const Uint dim=elem_coords.cols();
 
  
  for(Uint node = 0; node != nb_nodes; ++node)
    for (Uint d=0; d<dim; ++d)
      elem_coords(node,d) = coords_table[elem_nodes[node]][d];
}

////////////////////////////////////////////////////////////////////////////////

CTable<Uint>::ConstRow CElements::get_nodes(const Uint elem_idx) const
{
  return node_connectivity()[elem_idx];
//  CTable<Uint>::ConstRow elem_nodes = connectivity_table(space)[elem_idx];
//  return std::vector<Uint> (elem_nodes.begin(),elem_nodes.end());
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
