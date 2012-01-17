// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "common/Log.hpp"
#include "common/Link.hpp"
#include "common/Group.hpp"
#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

#include "mesh/Elements.hpp"
#include "mesh/Connectivity.hpp"
#include "common/List.hpp"
#include "mesh/ElementData.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/Space.hpp"

namespace cf3 {
namespace mesh {

using namespace common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Elements, Entities, LibMesh > Elements_Builder;

////////////////////////////////////////////////////////////////////////////////

Elements::Elements ( const std::string& name ) :
  Entities ( name )
{
  properties()["brief"] = std::string("Holds information of elements of one type");
  properties()["description"] = std::string("Container component that stores the element to node connectivity,\n")
  +std::string("a link to node storage, a list of used nodes, and global numbering unique over all processors");
}

////////////////////////////////////////////////////////////////////////////////

Elements::~Elements()
{
}

//////////////////////////////////////////////////////////////////////////////

Uint Elements::size() const
{
  return node_connectivity().size();
}

//////////////////////////////////////////////////////////////////////////////

Connectivity& Elements::node_connectivity() const
{
  return geometry_space().connectivity();
}

////////////////////////////////////////////////////////////////////////////////

RealMatrix Elements::get_coordinates(const Uint elem_idx) const
{
  const common::Table<Real>& coords_table = geometry_fields().coordinates();
  Connectivity::ConstRow elem_nodes = node_connectivity()[elem_idx];

  const Uint nb_nodes=elem_nodes.size();
  const Uint dim=coords_table.row_size();
  RealMatrix elem_coords(nb_nodes,dim);

  put_coordinates(elem_coords, elem_idx);

  return elem_coords;
}

////////////////////////////////////////////////////////////////////////////////

void Elements::put_coordinates(RealMatrix& elem_coords, const Uint elem_idx) const
{
  const common::Table<Real>& coords_table = geometry_fields().coordinates();
  Connectivity::ConstRow elem_nodes = node_connectivity()[elem_idx];

  fill(elem_coords, coords_table, elem_nodes);
}

////////////////////////////////////////////////////////////////////////////////

common::Table<Uint>::ConstRow Elements::get_nodes(const Uint elem_idx) const
{
  cf3_assert_desc( to_str(elem_idx)+ ">="+to_str(node_connectivity().size()) , elem_idx < node_connectivity().size() );
  return node_connectivity()[elem_idx];
//  common::Table<Uint>::ConstRow elem_nodes = connectivity_table(space)[elem_idx];
//  return std::vector<Uint> (elem_nodes.begin(),elem_nodes.end());
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
