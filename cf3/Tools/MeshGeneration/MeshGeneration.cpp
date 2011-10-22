// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign.hpp>

#include "MeshGeneration.hpp"

#include "common/Core.hpp"
#include "common/EventHandler.hpp"
#include "common/OptionURI.hpp"
#include "common/XML/SignalFrame.hpp"
#include "common/XML/SignalOptions.hpp"

#include "mesh/Region.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Faces.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Table.hpp"
#include "mesh/Geometry.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;
using namespace cf3::mesh;
using namespace cf3::mesh::BlockMesh;
using namespace cf3::math;
using namespace cf3::math::Consts;

using namespace boost::assign;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Tools {
namespace MeshGeneration {

/// Helper function to raise the mesh_loaded event and update mesh statustics
/// This must be called at the end of every mesh generation method
void mesh_loaded(Mesh& mesh)
{
  mesh.update_statistics();
  // Raise an event to indicate that a mesh was loaded happened
  SignalOptions options;
  options.add_option< OptionURI >("mesh_uri", mesh.uri());

  SignalFrame f= options.create_frame();
  Core::instance().event_handler().raise_event( "mesh_loaded", f );
}

////////////////////////////////////////////////////////////////////////////////

/// Helper function to build the GIDS in a serial mesh
void build_serial_gids(Mesh& mesh)
{
  const Uint nb_nodes = mesh.geometry().size();

  List<Uint>& gids = mesh.geometry().glb_idx(); gids.resize(nb_nodes);
  List<Uint>& ranks = mesh.geometry().rank(); ranks.resize(nb_nodes);
  for(Uint i = 0; i != nb_nodes; ++i)
  {
    ranks[i] = 0;
    gids[i] = i;
  }
}

////////////////////////////////////////////////////////////////////////////////

void create_line(Mesh& mesh, const Real x_len, const Uint x_segments)
{
  Region& region = mesh.topology().create_region("fluid");
  Geometry& nodes = mesh.geometry();
  mesh.initialize_nodes(x_segments+1,DIM_1D);
  const Real x_step = x_len / static_cast<Real>(x_segments);
  for(Uint i = 0; i <= x_segments; ++i)
  {
    nodes.coordinates()[i][XX] = static_cast<Real>(i) * x_step;
  }

  Cells::Ptr cells = region.create_component_ptr<Cells>("Line");
  cells->initialize("cf3.mesh.LagrangeP1.Line1D",nodes);
  Table<Uint>& connectivity = cells->node_connectivity();
  connectivity.resize(x_segments);
  for(Uint i = 0; i < x_segments; ++i)
  {
    Table<Uint>::Row nodes = connectivity[i];
    nodes[0] = i;
    nodes[1] = i+1;
  }

  // Left boundary point
  Faces::Ptr xneg = mesh.topology().create_region("xneg").create_component_ptr<Faces>("Point");
  xneg->initialize("cf3.mesh.LagrangeP0.Point1D", nodes);
  Table<Uint>& xneg_connectivity = xneg->node_connectivity();
  xneg_connectivity.resize(1);
  xneg_connectivity[0][0] = 0;

  // right boundary point
  Faces::Ptr xpos = mesh.topology().create_region("xpos").create_component_ptr<Faces>("Point");
  xpos->initialize("cf3.mesh.LagrangeP0.Point1D", nodes);
  Table<Uint>& xpos_connectivity = xpos->node_connectivity();
  xpos_connectivity.resize(1);
  xpos_connectivity[0][0] = x_segments;

  build_serial_gids(mesh);
  mesh_loaded(mesh);
}


void create_rectangle(Mesh& mesh, const Real x_len, const Real y_len, const Uint x_segments, const Uint y_segments)
{
  Region& region = mesh.topology().create_region("region");
  Geometry& nodes = mesh.geometry();
  mesh.initialize_nodes((x_segments+1)*(y_segments+1),DIM_2D);

  const Real x_step = x_len / static_cast<Real>(x_segments);
  const Real y_step = y_len / static_cast<Real>(y_segments);
  Real y;
  for(Uint j = 0; j <= y_segments; ++j)
  {
    y = static_cast<Real>(j) * y_step;
    for(Uint i = 0; i <= x_segments; ++i)
    {
      Table<Real>::Row row = nodes.coordinates()[j*(x_segments+1)+i];
      row[XX] = static_cast<Real>(i) * x_step;
      row[YY] = y;
    }
  }

  Cells::Ptr cells = region.create_component_ptr<Cells>("Quad");
  cells->initialize("cf3.mesh.LagrangeP1.Quad2D",nodes);
  Table<Uint>& connectivity = cells->node_connectivity();
  connectivity.resize((x_segments)*(y_segments));
  for(Uint j = 0; j < y_segments; ++j)
  {
    for(Uint i = 0; i < x_segments; ++i)
    {
      Table<Uint>::Row nodes = connectivity[j*(x_segments)+i];
      nodes[0] = j * (x_segments+1) + i;
      nodes[1] = nodes[0] + 1;
      nodes[3] = (j+1) * (x_segments+1) + i;
      nodes[2] = nodes[3] + 1;
    }
  }

  Faces::Ptr left = mesh.topology().create_region("left").create_component_ptr<Faces>("Line");
  left->initialize("cf3.mesh.LagrangeP1.Line2D", nodes);
  Table<Uint>& left_connectivity = left->node_connectivity();
  left_connectivity.resize(y_segments);
  for(Uint j = 0; j < y_segments; ++j)
  {
    Table<Uint>::Row crow = left_connectivity[j];
    crow[0] = j * (x_segments+1);
    crow[1] = (j+1) * (x_segments+1);
  }

  Faces::Ptr right = mesh.topology().create_region("right").create_component_ptr<Faces>("Line");
  right->initialize("cf3.mesh.LagrangeP1.Line2D", nodes);
  Table<Uint>& right_connectivity = right->node_connectivity();
  right_connectivity.resize(y_segments);
  for(Uint j = 0; j < y_segments; ++j)
  {
    Table<Uint>::Row nodes = right_connectivity[j];
    nodes[1] = j * (x_segments+1) + x_segments;
    nodes[0] = (j+1) * (x_segments+1) + x_segments;
  }

  Faces::Ptr bottom = mesh.topology().create_region("bottom").create_component_ptr<Faces>("Line");
  bottom->initialize("cf3.mesh.LagrangeP1.Line2D", nodes);
  Table<Uint>& bottom_connectivity = bottom->node_connectivity();
  bottom_connectivity.resize(x_segments);
  for(Uint i = 0; i < x_segments; ++i)
  {
    Table<Uint>::Row nodes = bottom_connectivity[i];
    nodes[0] = i;
    nodes[1] = i+1;
  }

  Faces::Ptr top = mesh.topology().create_region("top").create_component_ptr<Faces>("Line");
  top->initialize("cf3.mesh.LagrangeP1.Line2D", nodes);
  Table<Uint>& top_connectivity = top->node_connectivity();
  top_connectivity.resize(x_segments);
  for(Uint i = 0; i < x_segments; ++i)
  {
    Table<Uint>::Row nodes = top_connectivity[i];
    nodes[1] = y_segments * (x_segments+1) + i;
    nodes[0] = nodes[1] + 1;
  }

  Faces::Ptr center = mesh.topology().create_region("center_line").create_component_ptr<Faces>("Line");
  center->initialize("cf3.mesh.LagrangeP1.Line2D", nodes);
  Table<Uint>& center_connectivity = center->node_connectivity();
  center_connectivity.resize(y_segments);
  for(Uint j = 0; j < y_segments; ++j)
  {
    Table<Uint>::Row crow = center_connectivity[j];
    crow[0] = j * (x_segments+1) + x_segments/2;
    crow[1] = (j+1) * (x_segments+1) + x_segments/2;
  }

  Elements::Ptr corner = mesh.topology().create_region("corner").create_component_ptr<Elements>("Point");
  corner->initialize("cf3.mesh.LagrangeP0.Point2D",nodes);
  Table<Uint>& corner_connectivity = corner->node_connectivity();
  corner_connectivity.resize(1);
  corner_connectivity[0][0] = 0;

  Elements::Ptr center_point = mesh.topology().create_region("center_point").create_component_ptr<Elements>("Point");
  center_point->initialize("cf3.mesh.LagrangeP0.Point2D",nodes);
  Table<Uint>& center_point_connectivity = center_point->node_connectivity();
  center_point_connectivity.resize(1);
  center_point_connectivity[0][0] = y_segments/2 * (x_segments+1) + x_segments/2;

  build_serial_gids(mesh);
  mesh_loaded(mesh);
}

void create_rectangle_tris(Mesh& mesh, const Real x_len, const Real y_len, const Uint x_segments, const Uint y_segments)
{
  Region& region = mesh.topology().create_region("region");
  Geometry& nodes = mesh.geometry();
  mesh.initialize_nodes((x_segments+1)*(y_segments+1),DIM_2D);

  const Real x_step = x_len / static_cast<Real>(x_segments);
  const Real y_step = y_len / static_cast<Real>(y_segments);
  Real y;
  for(Uint j = 0; j <= y_segments; ++j)
  {
    y = static_cast<Real>(j) * y_step;
    for(Uint i = 0; i <= x_segments; ++i)
    {
      Table<Real>::Row row = nodes.coordinates()[j*(x_segments+1)+i];
      row[XX] = static_cast<Real>(i) * x_step;
      row[YY] = y;
    }
  }

  Cells::Ptr cells = region.create_component_ptr<Cells>("Triag");
  cells->initialize("cf3.mesh.LagrangeP1.Triag2D",nodes);
  Table<Uint>& connectivity = cells->node_connectivity();
  connectivity.resize(2*(x_segments)*(y_segments));
  for(Uint j = 0; j < y_segments; ++j)
  {
    for(Uint i = 0; i < x_segments; ++i)
    {
      const Uint node0 = j * (x_segments+1) + i;
      const Uint node1 = node0 + 1;
      const Uint node3 = (j+1) * (x_segments+1) + i;
      const Uint node2 = node3 + 1;

      // Upper triangle nodes
      Table<Uint>::Row nodes_u = connectivity[2*(j*(x_segments)+i)];
      nodes_u[0] = node0;
      nodes_u[1] = node2;
      nodes_u[2] = node3;

      // Lower triangle nodes
      Table<Uint>::Row nodes_l = connectivity[2*(j*(x_segments)+i)+1];
      nodes_l[0] = node0;
      nodes_l[1] = node1;
      nodes_l[2] = node2;
    }
  }

  Faces::Ptr left = mesh.topology().create_region("left").create_component_ptr<Faces>("Line");
  left->initialize("cf3.mesh.LagrangeP1.Line2D", nodes);
  Table<Uint>& left_connectivity = left->node_connectivity();
  left_connectivity.resize(y_segments);
  for(Uint j = 0; j < y_segments; ++j)
  {
    Table<Uint>::Row crow = left_connectivity[j];
    crow[0] = j * (x_segments+1);
    crow[1] = (j+1) * (x_segments+1);
  }

  Faces::Ptr right = mesh.topology().create_region("right").create_component_ptr<Faces>("Line");
  right->initialize("cf3.mesh.LagrangeP1.Line2D", nodes);
  Table<Uint>& right_connectivity = right->node_connectivity();
  right_connectivity.resize(y_segments);
  for(Uint j = 0; j < y_segments; ++j)
  {
    Table<Uint>::Row nodes = right_connectivity[j];
    nodes[1] = j * (x_segments+1) + x_segments;
    nodes[0] = (j+1) * (x_segments+1) + x_segments;
  }

  Faces::Ptr bottom = mesh.topology().create_region("bottom").create_component_ptr<Faces>("Line");
  bottom->initialize("cf3.mesh.LagrangeP1.Line2D", nodes);
  Table<Uint>& bottom_connectivity = bottom->node_connectivity();
  bottom_connectivity.resize(x_segments);
  for(Uint i = 0; i < x_segments; ++i)
  {
    Table<Uint>::Row nodes = bottom_connectivity[i];
    nodes[0] = i;
    nodes[1] = i+1;
  }

  Faces::Ptr top = mesh.topology().create_region("top").create_component_ptr<Faces>("Line");
  top->initialize("cf3.mesh.LagrangeP1.Line2D", nodes);
  Table<Uint>& top_connectivity = top->node_connectivity();
  top_connectivity.resize(x_segments);
  for(Uint i = 0; i < x_segments; ++i)
  {
    Table<Uint>::Row nodes = top_connectivity[i];
    nodes[1] = y_segments * (x_segments+1) + i;
    nodes[0] = nodes[1] + 1;
  }

  Faces::Ptr center = mesh.topology().create_region("center_line").create_component_ptr<Faces>("Line");
  center->initialize("cf3.mesh.LagrangeP1.Line2D", nodes);
  Table<Uint>& center_connectivity = center->node_connectivity();
  center_connectivity.resize(y_segments);
  for(Uint j = 0; j < y_segments; ++j)
  {
    Table<Uint>::Row crow = center_connectivity[j];
    crow[0] = j * (x_segments+1) + x_segments/2;
    crow[1] = (j+1) * (x_segments+1) + x_segments/2;
  }

  Elements::Ptr corner = mesh.topology().create_region("corner").create_component_ptr<Elements>("Point");
  corner->initialize("cf3.mesh.LagrangeP0.Point2D",nodes);
  Table<Uint>& corner_connectivity = corner->node_connectivity();
  corner_connectivity.resize(1);
  corner_connectivity[0][0] = 0;

  Elements::Ptr center_point = mesh.topology().create_region("center_point").create_component_ptr<Elements>("Point");
  center_point->initialize("cf3.mesh.LagrangeP0.Point2D",nodes);
  Table<Uint>& center_point_connectivity = center_point->node_connectivity();
  center_point_connectivity.resize(1);
  center_point_connectivity[0][0] = y_segments/2 * (x_segments+1) + x_segments/2;

  build_serial_gids(mesh);
  mesh_loaded(mesh);
}


/*
void create_circle_2d(Table<Real>& coordinates, Table<Uint>& connectivity, const Real radius, const Uint segments, const Real start_angle, const Real end_angle)
{
  const Uint dim = 2;
  const Uint nb_nodes = 2;
  const bool closed = std::abs(std::abs(end_angle - start_angle) - 2.0*pi()) < eps();

  coordinates.set_row_size(dim);
  Table<Real>::ArrayT& coord_array = coordinates.array();
  coord_array.resize(boost::extents[segments + (!closed)][dim]);

  connectivity.set_row_size(nb_nodes);
  Table<Uint>::ArrayT& conn_array = connectivity.array();
  conn_array.resize(boost::extents[segments][nb_nodes]);
  for(Uint u = 0; u != segments; ++u)
  {
    const Real theta = start_angle + (end_angle - start_angle) * (static_cast<Real>(u) / static_cast<Real>(segments));
    Table<Real>::Row coord_row = coord_array[u];

    coord_row[XX] = radius * cos(theta);
    coord_row[YY] = radius * sin(theta);

    Table<Uint>::Row nodes = conn_array[u];
    nodes[0] = u;
    nodes[1] = u+1;
  }
  if(closed)
  {
    conn_array[segments-1][1] = 0;
  }
  else
  {
    Table<Real>::Row coord_row = coord_array[segments];
    coord_row[XX] = radius * cos(end_angle);
    coord_row[YY] = radius * sin(end_angle);
  }
}*/

void create_circle_2d ( Mesh& mesh, const Real radius, const Uint segments, const Real start_angle, const Real end_angle )
{
  Region& region = mesh.topology().create_region("region");
  Geometry& nodes = mesh.geometry();

  Faces::Ptr cells = region.create_component_ptr<Faces>("Faces");
  cells->initialize("cf3.mesh.LagrangeP1.Line2D",nodes);
  Table<Uint>& connectivity = cells->node_connectivity();

  const bool closed = std::abs(std::abs(end_angle - start_angle) - 2.0*pi()) < eps();

  mesh.initialize_nodes(segments + Uint(!closed) , DIM_2D);
  connectivity.resize(segments);

  for(Uint u = 0; u != segments; ++u)
  {
    const Real theta = start_angle + (end_angle - start_angle) * (static_cast<Real>(u) / static_cast<Real>(segments));
    Table<Real>::Row coord_row = nodes.coordinates()[u];

    coord_row[XX] = radius * cos(theta);
    coord_row[YY] = radius * sin(theta);

    Table<Uint>::Row nodes = connectivity[u];
    nodes[0] = u;
    nodes[1] = u+1;
  }
  if(closed)
  {
    connectivity[segments-1][1] = 0;
  }
  else
  {
    Table<Real>::Row coord_row = nodes.coordinates()[segments];
    coord_row[XX] = radius * cos(end_angle);
    coord_row[YY] = radius * sin(end_angle);
  }
  build_serial_gids(mesh);
  mesh_loaded(mesh);
}

void create_channel_3d(BlockData& blocks, const Real length, const Real half_height, const Real width, const Uint x_segs, const Uint y_segs_half, const Uint z_segs, const Real ratio)
{
  blocks.scaling_factor = 1.;
  blocks.dimension = 3;

  blocks.points += list_of(0.    )(-half_height)(0.   )
                 , list_of(length)(-half_height)(0.   )
                 , list_of(0.    )( 0.         )(0.   )
                 , list_of(length)( 0.         )(0.   )
                 , list_of(0.    )( half_height)(0.   )
                 , list_of(length)( half_height)(0.   )
                 , list_of(0.    )(-half_height)(width)
                 , list_of(length)(-half_height)(width)
                 , list_of(0.    )( 0.         )(width)
                 , list_of(length)( 0.         )(width)
                 , list_of(0.    )( half_height)(width)
                 , list_of(length)( half_height)(width);

  blocks.block_points += list_of(0)(1)(3)(2)(6)(7)(9)(8)
                       , list_of(2)(3)(5)(4)(8)(9)(11)(10);
  blocks.block_subdivisions += list_of(x_segs)(y_segs_half)(z_segs)
                             , list_of(x_segs)(y_segs_half)(z_segs);
  blocks.block_gradings += list_of(1.)(1.)(1.)(1.)(1./ratio)(1./ratio)(1./ratio)(1./ratio)(1.)(1.)(1.)(1.)
                         , list_of(1.)(1.)(1.)(1.)(ratio   )(ratio   )(ratio   )(ratio   )(1.)(1.)(1.)(1.);
  blocks.block_distribution += 0, 2;

  blocks.patch_names += "bottomWall", "topWall", "sides1", "sides2", "inout1", "inout2";
  blocks.patch_types += "wall"      , "wall"   , "cyclic", "cyclic", "cyclic", "cyclic";
  blocks.patch_points += list_of(0)(1)(7)(6),
                         list_of(4)(10)(11)(5),
                         list_of(0)(2)(3)(1)(6)(7)(9)(8),
                         list_of(2)(4)(5)(3)(8)(9)(11)(10),
                         list_of(0)(6)(8)(2)(1)(3)(9)(7),
                         list_of(2)(8)(10)(4)(3)(5)(11)(9);

}


////////////////////////////////////////////////////////////////////////////////

} // MeshGeneration
} // Tools
} // cf3

////////////////////////////////////////////////////////////////////////////////


