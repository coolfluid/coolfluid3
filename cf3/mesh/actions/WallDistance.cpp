// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>

#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Option.hpp"
#include "common/OptionList.hpp"

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

#include "mesh/LagrangeP1/Triag2D.hpp"
#include "mesh/LagrangeP1/Quad2D.hpp"

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
    std::vector<Uint> neighbor_nodes; // Collect neighboring nodes, so we can project onto a sharp corner in 3D if needed (i.e. near a step)
    // Loop over all surface elements around the given node
    BOOST_FOREACH(const Uint elem_idx, m_node_connectivity.node_element_range(surface_node_idx))
    {
      // Get the element coordinates
      const CNodeConnectivity::ElementReferenceT element_ref = m_node_connectivity.element(elem_idx);
      const ElementType& etype = element_ref.first->element_type();
      const Uint element_nb_nodes = etype.nb_nodes();
      elem_coords.resize(element_nb_nodes, dim);
      const Connectivity::ConstRow conn_row = element_ref.first->geometry_space().connectivity()[element_ref.second];
      fill(elem_coords, m_coords, conn_row);

      // We consider lines, triangles and quads as viable surface elements
      if(element_nb_nodes < 2 || element_nb_nodes > 4 || etype.order() != 1)
      {
        throw common::SetupError(FromHere(), "Unsupported surface element of type " + etype.name() + " in surface region " + element_ref.first->uri().path());
      }
      
      bool in_element = false;

      if(element_nb_nodes == 2) // line segment
      {
        cf3_assert(dim == 2);
        cf3_assert(etype.shape() == GeoShape::LINE);
        RealVector e1 = elem_coords.row(1) - elem_coords.row(0); // line segment vector
        Real e1_len = e1.norm();
        e1 /= e1_len;
        const Real projection = e1.dot(inner_coord - elem_coords.row(0).transpose());
        // If the projection of the node along the normal fits inside the element, we can take the normal distance
        in_element = projection > 0 && projection < e1_len;
      }
      if(element_nb_nodes == 3)
      {
        cf3_assert(dim == 3);
        cf3_assert(etype.shape() == GeoShape::TRIAG);
        RealVector3 e1 = (elem_coords.row(1) - elem_coords.row(0)).normalized();
        RealVector3 en = elem_coords.row(2) - elem_coords.row(0);
        RealVector3 e2 = (e1.cross(en)).cross(e1).normalized();
        RealVector3 p = inner_coord - elem_coords.row(0).transpose();
        
        // Construct 2D coordinates for the boundary element
        Eigen::Matrix<Real, 3, 2> triag_coords_2d;
        triag_coords_2d.row(0).setZero();
        triag_coords_2d(1,0) = e1.dot(elem_coords.row(1) - elem_coords.row(0));
        triag_coords_2d(1,1) = 0.;
        triag_coords_2d(2,0) = e1.dot(elem_coords.row(2) - elem_coords.row(0));
        triag_coords_2d(2,1) = e2.dot(elem_coords.row(2) - elem_coords.row(0));
        
        RealVector2 p_proj(2);
        p_proj[0] = p.dot(e1);
        p_proj[1] = p.dot(e2);
        
        in_element = LagrangeP1::Triag2D::is_coord_in_element(p_proj, triag_coords_2d);
        const Uint origin_corner = std::find(conn_row.begin(), conn_row.end(), surface_node_idx) - conn_row.begin();
        if(origin_corner == 0)
        {
          neighbor_nodes.push_back(conn_row[1]);
          neighbor_nodes.push_back(conn_row[2]);
        }
        else if(origin_corner == 1)
        {
          neighbor_nodes.push_back(conn_row[0]);
          neighbor_nodes.push_back(conn_row[2]);
        }
        else
        {
          neighbor_nodes.push_back(conn_row[0]);
          neighbor_nodes.push_back(conn_row[1]);
        }
      }
      if(element_nb_nodes == 4)
      {
        cf3_assert(dim == 3);
        cf3_assert(etype.shape() == GeoShape::QUAD);
        RealVector3 e1 = (elem_coords.row(1) - elem_coords.row(0)).normalized();
        RealVector3 en = elem_coords.row(3) - elem_coords.row(0);
        RealVector3 e2 = (e1.cross(en)).cross(e1).normalized();
        RealVector3 p = inner_coord - elem_coords.row(0).transpose();

        // Construct 2D coordinates for the boundary element
        Eigen::Matrix<Real, 4, 2> quad_coords_2d;
        quad_coords_2d.row(0).setZero();
        for(int i = 1; i != 4; ++i)
        {
          quad_coords_2d(i, 0) = e1.dot(elem_coords.row(i) - elem_coords.row(0));
          quad_coords_2d(i, 1) = e2.dot(elem_coords.row(i) - elem_coords.row(0));
        }

        RealVector2 p_proj(2);
        p_proj[0] = p.dot(e1);
        p_proj[1] = p.dot(e2);

        in_element = LagrangeP1::Quad2D::is_coord_in_element(p_proj, quad_coords_2d);
        const Uint origin_corner = std::find(conn_row.begin(), conn_row.end(), surface_node_idx) - conn_row.begin();
        if(origin_corner == 0 || origin_corner == 2)
        {
          neighbor_nodes.push_back(conn_row[1]);
          neighbor_nodes.push_back(conn_row[3]);
        }
        else
        {
          neighbor_nodes.push_back(conn_row[0]);
          neighbor_nodes.push_back(conn_row[2]);
        }
      }
      
      // If the projection was in an element, we can just proceed to compute the normal distance
      if(in_element)
      {
        RealVector n(dim); // normal vector
        etype.compute_normal(elem_coords, n);
        return fabs((n/n.norm()).dot(inner_coord - elem_coords.row(0).transpose()));
      }
    }
    // If we got here, no projections on the elements gave a result
    // First, verify the 3D case where we need to project on "step" edges
    BOOST_FOREACH(const Uint neighbor_node, neighbor_nodes)
    {
      const RealVector surface_coord = to_vector(m_coords[surface_node_idx]);
      const RealVector neighbor_coord = to_vector(m_coords[neighbor_node]);
      RealVector e1 = neighbor_coord - surface_coord;
      Real e1_len = e1.norm();
      e1 /= e1_len;
      const Real projection = e1.dot(inner_coord - surface_coord);
      if(projection > 0 && projection < e1_len)
      {
        return (inner_coord - (surface_coord + e1*projection)).norm();
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
  options().add("regions", m_regions)
      .pretty_name("Regions")
      .description("Regions that are to be considered as part of the wall")
      .link_to(&m_regions)
      .mark_basic();
}

void WallDistance::execute()
{
  Mesh& mesh = *m_mesh;

  Field& d = mesh.geometry_fields().create_field("WallDistance", "wall_distance");
  const Field& coords = mesh.geometry_fields().coordinates();
  const Uint nb_nodes = coords.size();

  boost::shared_ptr<CNodeConnectivity> node_connectivity = common::allocate_component<CNodeConnectivity>("NodeConnectivity");
  std::vector< Handle<Entities const> > surface_entities;
  BOOST_FOREACH(const Handle<Region const>& region, m_regions)
  {
    BOOST_FOREACH(const mesh::Elements& elements, common::find_components_recursively_with_filter<mesh::Elements>(*region, IsElementsSurface()))
    {
      surface_entities.push_back(elements.handle<Entities const>());
    }
  }

  node_connectivity->initialize(nb_nodes, surface_entities);

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
