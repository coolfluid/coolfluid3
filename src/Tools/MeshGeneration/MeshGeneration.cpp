// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign.hpp>

#include "MeshGeneration.hpp"

#include "Common/Core.hpp"
#include "Common/EventHandler.hpp"
#include "Common/OptionURI.hpp"
#include "Common/XML/SignalFrame.hpp"
#include "Common/XML/SignalOptions.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CFaces.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/Geometry.hpp"

using namespace CF::Common;
using namespace CF::Common::XML;
using namespace CF::Mesh;
using namespace CF::Mesh::BlockMesh;
using namespace CF::Math;
using namespace CF::Math::Consts;

using namespace boost::assign;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {
namespace MeshGeneration {

/// Helper function to raise the mesh_loaded event and update mesh statustics
/// This must be called at the end of every mesh generation method
void mesh_loaded(CMesh& mesh)
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
void build_serial_gids(CMesh& mesh)
{
  const Uint nb_nodes = mesh.geometry().size();

  CList<Uint>& gids = mesh.geometry().glb_idx(); gids.resize(nb_nodes);
  CList<Uint>& ranks = mesh.geometry().rank(); ranks.resize(nb_nodes);
  for(Uint i = 0; i != nb_nodes; ++i)
  {
    ranks[i] = 0;
    gids[i] = i;
  }
}

////////////////////////////////////////////////////////////////////////////////

void create_line(CMesh& mesh, const Real x_len, const Uint x_segments)
{
  CRegion& region = mesh.topology().create_region("fluid");
  Geometry& nodes = mesh.geometry();
  mesh.initialize_nodes(x_segments+1,DIM_1D);
  const Real x_step = x_len / static_cast<Real>(x_segments);
  for(Uint i = 0; i <= x_segments; ++i)
  {
    nodes.coordinates()[i][XX] = static_cast<Real>(i) * x_step;
  }

  CCells::Ptr cells = region.create_component_ptr<CCells>("Line");
  cells->initialize("CF.Mesh.LagrangeP1.Line1D",nodes);
  CTable<Uint>& connectivity = cells->node_connectivity();
  connectivity.resize(x_segments);
  for(Uint i = 0; i < x_segments; ++i)
  {
    CTable<Uint>::Row nodes = connectivity[i];
    nodes[0] = i;
    nodes[1] = i+1;
  }

  // Left boundary point
  CFaces::Ptr xneg = mesh.topology().create_region("xneg").create_component_ptr<CFaces>("Point");
  xneg->initialize("CF.Mesh.LagrangeP0.Point1D", nodes);
  CTable<Uint>& xneg_connectivity = xneg->node_connectivity();
  xneg_connectivity.resize(1);
  xneg_connectivity[0][0] = 0;

  // right boundary point
  CFaces::Ptr xpos = mesh.topology().create_region("xpos").create_component_ptr<CFaces>("Point");
  xpos->initialize("CF.Mesh.LagrangeP0.Point1D", nodes);
  CTable<Uint>& xpos_connectivity = xpos->node_connectivity();
  xpos_connectivity.resize(1);
  xpos_connectivity[0][0] = x_segments;

  build_serial_gids(mesh);
  mesh_loaded(mesh);
}


void create_rectangle(CMesh& mesh, const Real x_len, const Real y_len, const Uint x_segments, const Uint y_segments)
{
  CRegion& region = mesh.topology().create_region("region");
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
      CTable<Real>::Row row = nodes.coordinates()[j*(x_segments+1)+i];
      row[XX] = static_cast<Real>(i) * x_step;
      row[YY] = y;
    }
  }

  CCells::Ptr cells = region.create_component_ptr<CCells>("Quad");
  cells->initialize("CF.Mesh.LagrangeP1.Quad2D",nodes);
  CTable<Uint>& connectivity = cells->node_connectivity();
  connectivity.resize((x_segments)*(y_segments));
  for(Uint j = 0; j < y_segments; ++j)
  {
    for(Uint i = 0; i < x_segments; ++i)
    {
      CTable<Uint>::Row nodes = connectivity[j*(x_segments)+i];
      nodes[0] = j * (x_segments+1) + i;
      nodes[1] = nodes[0] + 1;
      nodes[3] = (j+1) * (x_segments+1) + i;
      nodes[2] = nodes[3] + 1;
    }
  }

  CFaces::Ptr left = mesh.topology().create_region("left").create_component_ptr<CFaces>("Line");
  left->initialize("CF.Mesh.LagrangeP1.Line2D", nodes);
  CTable<Uint>& left_connectivity = left->node_connectivity();
  left_connectivity.resize(y_segments);
  for(Uint j = 0; j < y_segments; ++j)
  {
    CTable<Uint>::Row crow = left_connectivity[j];
    crow[0] = j * (x_segments+1);
    crow[1] = (j+1) * (x_segments+1);
  }

  CFaces::Ptr right = mesh.topology().create_region("right").create_component_ptr<CFaces>("Line");
  right->initialize("CF.Mesh.LagrangeP1.Line2D", nodes);
  CTable<Uint>& right_connectivity = right->node_connectivity();
  right_connectivity.resize(y_segments);
  for(Uint j = 0; j < y_segments; ++j)
  {
    CTable<Uint>::Row nodes = right_connectivity[j];
    nodes[1] = j * (x_segments+1) + x_segments;
    nodes[0] = (j+1) * (x_segments+1) + x_segments;
  }

  CFaces::Ptr bottom = mesh.topology().create_region("bottom").create_component_ptr<CFaces>("Line");
  bottom->initialize("CF.Mesh.LagrangeP1.Line2D", nodes);
  CTable<Uint>& bottom_connectivity = bottom->node_connectivity();
  bottom_connectivity.resize(x_segments);
  for(Uint i = 0; i < x_segments; ++i)
  {
    CTable<Uint>::Row nodes = bottom_connectivity[i];
    nodes[0] = i;
    nodes[1] = i+1;
  }

  CFaces::Ptr top = mesh.topology().create_region("top").create_component_ptr<CFaces>("Line");
  top->initialize("CF.Mesh.LagrangeP1.Line2D", nodes);
  CTable<Uint>& top_connectivity = top->node_connectivity();
  top_connectivity.resize(x_segments);
  for(Uint i = 0; i < x_segments; ++i)
  {
    CTable<Uint>::Row nodes = top_connectivity[i];
    nodes[1] = y_segments * (x_segments+1) + i;
    nodes[0] = nodes[1] + 1;
  }

  CFaces::Ptr center = mesh.topology().create_region("center_line").create_component_ptr<CFaces>("Line");
  center->initialize("CF.Mesh.LagrangeP1.Line2D", nodes);
  CTable<Uint>& center_connectivity = center->node_connectivity();
  center_connectivity.resize(y_segments);
  for(Uint j = 0; j < y_segments; ++j)
  {
    CTable<Uint>::Row crow = center_connectivity[j];
    crow[0] = j * (x_segments+1) + x_segments/2;
    crow[1] = (j+1) * (x_segments+1) + x_segments/2;
  }

  CElements::Ptr corner = mesh.topology().create_region("corner").create_component_ptr<CElements>("Point");
  corner->initialize("CF.Mesh.LagrangeP0.Point2D",nodes);
  CTable<Uint>& corner_connectivity = corner->node_connectivity();
  corner_connectivity.resize(1);
  corner_connectivity[0][0] = 0;

  CElements::Ptr center_point = mesh.topology().create_region("center_point").create_component_ptr<CElements>("Point");
  center_point->initialize("CF.Mesh.LagrangeP0.Point2D",nodes);
  CTable<Uint>& center_point_connectivity = center_point->node_connectivity();
  center_point_connectivity.resize(1);
  center_point_connectivity[0][0] = y_segments/2 * (x_segments+1) + x_segments/2;

  build_serial_gids(mesh);
  mesh_loaded(mesh);
}

void create_rectangle_tris(CMesh& mesh, const Real x_len, const Real y_len, const Uint x_segments, const Uint y_segments)
{
  CRegion& region = mesh.topology().create_region("region");
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
      CTable<Real>::Row row = nodes.coordinates()[j*(x_segments+1)+i];
      row[XX] = static_cast<Real>(i) * x_step;
      row[YY] = y;
    }
  }

  CCells::Ptr cells = region.create_component_ptr<CCells>("Triag");
  cells->initialize("CF.Mesh.LagrangeP1.Triag2D",nodes);
  CTable<Uint>& connectivity = cells->node_connectivity();
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
      CTable<Uint>::Row nodes_u = connectivity[2*(j*(x_segments)+i)];
      nodes_u[0] = node0;
      nodes_u[1] = node2;
      nodes_u[2] = node3;

      // Lower triangle nodes
      CTable<Uint>::Row nodes_l = connectivity[2*(j*(x_segments)+i)+1];
      nodes_l[0] = node0;
      nodes_l[1] = node1;
      nodes_l[2] = node2;
    }
  }

  CFaces::Ptr left = mesh.topology().create_region("left").create_component_ptr<CFaces>("Line");
  left->initialize("CF.Mesh.LagrangeP1.Line2D", nodes);
  CTable<Uint>& left_connectivity = left->node_connectivity();
  left_connectivity.resize(y_segments);
  for(Uint j = 0; j < y_segments; ++j)
  {
    CTable<Uint>::Row crow = left_connectivity[j];
    crow[0] = j * (x_segments+1);
    crow[1] = (j+1) * (x_segments+1);
  }

  CFaces::Ptr right = mesh.topology().create_region("right").create_component_ptr<CFaces>("Line");
  right->initialize("CF.Mesh.LagrangeP1.Line2D", nodes);
  CTable<Uint>& right_connectivity = right->node_connectivity();
  right_connectivity.resize(y_segments);
  for(Uint j = 0; j < y_segments; ++j)
  {
    CTable<Uint>::Row nodes = right_connectivity[j];
    nodes[1] = j * (x_segments+1) + x_segments;
    nodes[0] = (j+1) * (x_segments+1) + x_segments;
  }

  CFaces::Ptr bottom = mesh.topology().create_region("bottom").create_component_ptr<CFaces>("Line");
  bottom->initialize("CF.Mesh.LagrangeP1.Line2D", nodes);
  CTable<Uint>& bottom_connectivity = bottom->node_connectivity();
  bottom_connectivity.resize(x_segments);
  for(Uint i = 0; i < x_segments; ++i)
  {
    CTable<Uint>::Row nodes = bottom_connectivity[i];
    nodes[0] = i;
    nodes[1] = i+1;
  }

  CFaces::Ptr top = mesh.topology().create_region("top").create_component_ptr<CFaces>("Line");
  top->initialize("CF.Mesh.LagrangeP1.Line2D", nodes);
  CTable<Uint>& top_connectivity = top->node_connectivity();
  top_connectivity.resize(x_segments);
  for(Uint i = 0; i < x_segments; ++i)
  {
    CTable<Uint>::Row nodes = top_connectivity[i];
    nodes[1] = y_segments * (x_segments+1) + i;
    nodes[0] = nodes[1] + 1;
  }

  CFaces::Ptr center = mesh.topology().create_region("center_line").create_component_ptr<CFaces>("Line");
  center->initialize("CF.Mesh.LagrangeP1.Line2D", nodes);
  CTable<Uint>& center_connectivity = center->node_connectivity();
  center_connectivity.resize(y_segments);
  for(Uint j = 0; j < y_segments; ++j)
  {
    CTable<Uint>::Row crow = center_connectivity[j];
    crow[0] = j * (x_segments+1) + x_segments/2;
    crow[1] = (j+1) * (x_segments+1) + x_segments/2;
  }

  CElements::Ptr corner = mesh.topology().create_region("corner").create_component_ptr<CElements>("Point");
  corner->initialize("CF.Mesh.LagrangeP0.Point2D",nodes);
  CTable<Uint>& corner_connectivity = corner->node_connectivity();
  corner_connectivity.resize(1);
  corner_connectivity[0][0] = 0;

  CElements::Ptr center_point = mesh.topology().create_region("center_point").create_component_ptr<CElements>("Point");
  center_point->initialize("CF.Mesh.LagrangeP0.Point2D",nodes);
  CTable<Uint>& center_point_connectivity = center_point->node_connectivity();
  center_point_connectivity.resize(1);
  center_point_connectivity[0][0] = y_segments/2 * (x_segments+1) + x_segments/2;

  build_serial_gids(mesh);
  mesh_loaded(mesh);
}


/*
void create_circle_2d(CTable<Real>& coordinates, CTable<Uint>& connectivity, const Real radius, const Uint segments, const Real start_angle, const Real end_angle)
{
  const Uint dim = 2;
  const Uint nb_nodes = 2;
  const bool closed = std::abs(std::abs(end_angle - start_angle) - 2.0*pi()) < eps();

  coordinates.set_row_size(dim);
  CTable<Real>::ArrayT& coord_array = coordinates.array();
  coord_array.resize(boost::extents[segments + (!closed)][dim]);

  connectivity.set_row_size(nb_nodes);
  CTable<Uint>::ArrayT& conn_array = connectivity.array();
  conn_array.resize(boost::extents[segments][nb_nodes]);
  for(Uint u = 0; u != segments; ++u)
  {
    const Real theta = start_angle + (end_angle - start_angle) * (static_cast<Real>(u) / static_cast<Real>(segments));
    CTable<Real>::Row coord_row = coord_array[u];

    coord_row[XX] = radius * cos(theta);
    coord_row[YY] = radius * sin(theta);

    CTable<Uint>::Row nodes = conn_array[u];
    nodes[0] = u;
    nodes[1] = u+1;
  }
  if(closed)
  {
    conn_array[segments-1][1] = 0;
  }
  else
  {
    CTable<Real>::Row coord_row = coord_array[segments];
    coord_row[XX] = radius * cos(end_angle);
    coord_row[YY] = radius * sin(end_angle);
  }
}*/

void create_circle_2d ( CMesh& mesh, const Real radius, const Uint segments, const Real start_angle, const Real end_angle )
{
  CRegion& region = mesh.topology().create_region("region");
  Geometry& nodes = mesh.geometry();

  CFaces::Ptr cells = region.create_component_ptr<CFaces>("Faces");
  cells->initialize("CF.Mesh.LagrangeP1.Line2D",nodes);
  CTable<Uint>& connectivity = cells->node_connectivity();

  const bool closed = std::abs(std::abs(end_angle - start_angle) - 2.0*pi()) < eps();

  mesh.initialize_nodes(segments + Uint(!closed) , DIM_2D);
  connectivity.resize(segments);

  for(Uint u = 0; u != segments; ++u)
  {
    const Real theta = start_angle + (end_angle - start_angle) * (static_cast<Real>(u) / static_cast<Real>(segments));
    CTable<Real>::Row coord_row = nodes.coordinates()[u];

    coord_row[XX] = radius * cos(theta);
    coord_row[YY] = radius * sin(theta);

    CTable<Uint>::Row nodes = connectivity[u];
    nodes[0] = u;
    nodes[1] = u+1;
  }
  if(closed)
  {
    connectivity[segments-1][1] = 0;
  }
  else
  {
    CTable<Real>::Row coord_row = nodes.coordinates()[segments];
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
} // CF

////////////////////////////////////////////////////////////////////////////////


