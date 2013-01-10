// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"

#include "mesh/ConnectivityData.hpp"
#include "mesh/DiscontinuousDictionary.hpp"
#include "mesh/ElementData.hpp"
#include "mesh/Faces.hpp"
#include "mesh/Region.hpp"
#include "mesh/Space.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"
#include "mesh/Functions.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/ElementData.hpp"

#include "WallDistance.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < WallDistance, MeshTransformer, mesh::actions::LibActions> WallDistance_Builder;

////////////////////////////////////////////////////////////////////////////////

namespace detail
{

/// Helper struct to handle projection to the wall near a given surface node
struct WallProjection
{
  WallProjection(const Field& coordinates, const CNodeConnectivity& node_connectivity) :
    m_coords(coordinates),
    m_node_connectivity(node_connectivity)
  {
  }

  // Get the wall distance for an inner node, looking at the elements that are adjacent to the given surface node
  Real operator()(const Uint inner_node_idx, const Uint surface_node_idx)
  {
    RealMatrix elem_coords;
    const Uint dim = m_coords.row_size();
    const RealVector inner_coord = to_vector(m_coords[inner_node_idx]);
    // Loop over all surface elements around the given node
    BOOST_FOREACH(const Uint elem_idx, m_node_connectivity.node_element_range(surface_node_idx))
    {
      // Get the element coordinates
      const CNodeConnectivity::ElementReferenceT element_ref = m_node_connectivity.element(elem_idx);
      const ElementType& etype = element_ref.first->element_type();
      const Uint element_nb_nodes = etype.nb_nodes();
      const Uint element_dim = etype.dimension();
      elem_coords.resize(element_nb_nodes, element_dim);
      fill(elem_coords, m_coords, element_ref.first->geometry_space().connectivity()[element_ref.second]);

      // We consider lines, triangles and quads as viable surface elements
      if(element_nb_nodes < 2 || element_nb_nodes > 4 || etype.order() != 1)
      {
        throw common::SetupError(FromHere(), "Unsupported surface element of type " + etype.name() + " in surface region " + element_ref.first->uri().path());
      }

      if(element_nb_nodes == 2) // line segment
      {
        RealVector n(2); // normal vector
        RealVector e1 = elem_coords.row(1) - elem_coords.row(0); // line segment vector
        Real e1_len = e1.norm();
        e1 /= e1_len;
        Real projection = e1.dot(inner_coord - elem_coords.row(0).transpose());
        // If the projection of the node along the normal fits inside the element, we can take the normal distance
        if(projection > 0 && projection < e1_len)
        {
          etype.compute_normal(elem_coords, n);
          return (n/n.norm()).dot(inner_coord - elem_coords.row(0).transpose());
        }
      }
    }
    return (inner_coord - to_vector(m_coords[surface_node_idx])).norm();
  }

  const Field& m_coords;
  const CNodeConnectivity& m_node_connectivity;
};
}

WallDistance::WallDistance(const std::string& name) : MeshTransformer(name)
{
}

void WallDistance::execute()
{
  Mesh& mesh = *m_mesh;

  Field& d = mesh.geometry_fields().create_field("WallDistance", "wall_distance");
  const Field& coords = mesh.geometry_fields().coordinates();
  const Uint nb_nodes = coords.size();

  boost::shared_ptr<CNodeConnectivity> node_connectivity = common::allocate_component<CNodeConnectivity>("NodeConnectivity");
  node_connectivity->initialize(nb_nodes, common::find_components_recursively_with_filter<mesh::Elements>(mesh.topology(), IsElementsSurface()));

  std::vector< Handle<Entities const> > surface_entities;
  BOOST_FOREACH(const mesh::Elements& elements, common::find_components_recursively_with_filter<mesh::Elements>(mesh.topology(), IsElementsSurface()))
  {
    surface_entities.push_back(elements.handle<Entities const>());
  }

  boost::shared_ptr< common::List< Uint > > surface_nodes_ptr = build_used_nodes_list(surface_entities, mesh.geometry_fields(), true);
  const common::List<Uint>& surface_nodes = *surface_nodes_ptr;
  const Uint nb_surface_nodes = surface_nodes.size();

  detail::WallProjection normal_distance(coords, *node_connectivity);

  for(Uint inner_node_idx = 0; inner_node_idx != nb_nodes; ++inner_node_idx)
  {
    bool is_surface_node = false;
    Real shortest_distance = 1e20;
    Uint closest_surface_node = 0;
    for(Uint j = 0; j != nb_surface_nodes; ++j)
    {
      const Uint surface_node_idx = surface_nodes[j];
      if(surface_node_idx == inner_node_idx)
      {
        is_surface_node = true;
        break;
      }
      const Real d2 = (to_vector(coords[inner_node_idx]) - to_vector(coords[surface_node_idx])).squaredNorm();
      if(d2 < shortest_distance)
      {
        shortest_distance = d2;
        closest_surface_node = surface_node_idx;
      }
    }

    d[inner_node_idx][0] = is_surface_node ? 0. : normal_distance(inner_node_idx, closest_surface_node);
  }

}

//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
