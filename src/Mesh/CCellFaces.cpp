// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Mesh/CCellFaces.hpp"
#include "Mesh/Geometry.hpp"
#include "Mesh/ElementType.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ComponentBuilder < CCellFaces, CEntities, LibMesh > CCellFaces_Builder;

////////////////////////////////////////////////////////////////////////////////

CCellFaces::CCellFaces ( const std::string& name ) :
  CEntities ( name )
{
  properties()["brief"] = std::string("Holds information of faces of one element type");
  properties()["description"] = std::string("Container component that stores the element to node connectivity,\n")
  +std::string("a link to node storage, and global numbering unique over all processors");

  m_cell_connectivity = create_static_component_ptr<CFaceCellConnectivity>("cell_connectivity");

  add_tag(Mesh::Tags::face_entity());
}

////////////////////////////////////////////////////////////////////////////////

CCellFaces::~CCellFaces()
{
}

////////////////////////////////////////////////////////////////////////////////

CTable<Uint>::ConstRow CCellFaces::get_nodes(const Uint face_idx) const
{
  m_proxy_nodes.resize(boost::extents[1][element_type().nb_nodes()]);

  std::vector<Uint> face_nodes = m_cell_connectivity->face_nodes(face_idx);
  for (Uint i=0; i<face_nodes.size(); ++i)
    m_proxy_nodes[0][i]=face_nodes[i];
  return m_proxy_nodes[0];
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix CCellFaces::get_coordinates(const Uint face_idx) const
{
  const CTable<Real>& coords_table = geometry().coordinates();
  std::vector<Uint> face_nodes = m_cell_connectivity->face_nodes(face_idx);

  const Uint nb_nodes=face_nodes.size();
  const Uint dim=coords_table.row_size();
  RealMatrix elem_coords(nb_nodes,dim);

  for(Uint node = 0; node != nb_nodes; ++node)
    for (Uint d=0; d<dim; ++d)
      elem_coords(node,d) = coords_table[face_nodes[node]][d];

  return elem_coords;

}

////////////////////////////////////////////////////////////////////////////////

void CCellFaces::put_coordinates(RealMatrix& elem_coords, const Uint face_idx) const
{
  const CTable<Real>& coords_table = geometry().coordinates();
  std::vector<Uint> face_nodes = m_cell_connectivity->face_nodes(face_idx);

  const Uint nb_nodes=elem_coords.rows();
  const Uint dim=elem_coords.cols();

  cf_assert(nb_nodes == face_nodes.size());
  cf_assert(dim==coords_table.row_size());

  for(Uint node = 0; node != nb_nodes; ++node)
    for (Uint d=0; d<dim; ++d)
      elem_coords(node,d) = coords_table[face_nodes[node]][d];

}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
