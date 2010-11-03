// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign.hpp>

#include "MeshGeneration.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CTable.hpp"


using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Mesh::BlockMesh;
using namespace CF::Math;

using namespace boost::assign;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {
namespace MeshGeneration {
  
////////////////////////////////////////////////////////////////////////////////

void create_line(CMesh& mesh, const Real x_len, const Uint x_segments)
{
  CRegion& region = mesh.create_region("region");
  CArray& coordinates = region.create_coordinates(1);
  CArray::ArrayT& coord_array = coordinates.array();
  
  coord_array.resize(boost::extents[(x_segments+1)][1]);
  const Real x_step = x_len / static_cast<Real>(x_segments);
  for(Uint i = 0; i <= x_segments; ++i)
  {
    coord_array[i][XX] = static_cast<Real>(i) * x_step;
  }
  
  CTable::ArrayT& conn_array = region.create_elements("Line1DLagrangeP1",coordinates).connectivity_table().array();
  conn_array.resize(boost::extents[x_segments][2]);
  for(Uint i = 0; i < x_segments; ++i)
  {
    CTable::Row nodes = conn_array[i];
    nodes[0] = i;
    nodes[1] = i+1;
  }
  
  // Left boundary
  CElements& xneg_elements = region.create_region("xneg").create_elements("Point1DLagrangeP1", coordinates);
  CTable::ArrayT& xneg_connectivity = xneg_elements.connectivity_table().array();
  xneg_connectivity.resize(boost::extents[1][1]);
  xneg_connectivity[0][0] = 0;
  
  // right boundary
  CElements& xpos_elements = region.create_region("xpos").create_elements("Point1DLagrangeP1", coordinates);
  CTable::ArrayT& xpos_connectivity = xpos_elements.connectivity_table().array();
  xpos_connectivity.resize(boost::extents[1][1]);
  xpos_connectivity[0][0] = x_segments + 1;
}


void create_rectangle(CMesh& mesh, const Real x_len, const Real y_len, const Uint x_segments, const Uint y_segments)
{
  CRegion& region = mesh.create_region("region");
  
  const Uint dim = 2;
  CArray& coordinates = region.create_coordinates(dim);
  CArray::ArrayT& coord_array = coordinates.array();
  coord_array.resize(boost::extents[(x_segments+1)*(y_segments+1)][dim]);
  const Real x_step = x_len / static_cast<Real>(x_segments);
  const Real y_step = y_len / static_cast<Real>(y_segments);
  Real y;
  for(Uint j = 0; j <= y_segments; ++j)
  {
    y = static_cast<Real>(j) * y_step;
    for(Uint i = 0; i <= x_segments; ++i)
    {
      CArray::Row row = coord_array[j*(x_segments+1)+i];
      row[XX] = static_cast<Real>(i) * x_step;
      row[YY] = y;
    }
  }
  CTable::ArrayT& conn_array = region.create_elements("Quad2DLagrangeP1",coordinates).connectivity_table().array();
  conn_array.resize(boost::extents[(x_segments)*(y_segments)][4]);
  for(Uint j = 0; j < y_segments; ++j)
  {
    for(Uint i = 0; i < x_segments; ++i)
    {
      CTable::Row nodes = conn_array[j*(x_segments)+i];
      nodes[0] = j * (x_segments+1) + i;
      nodes[1] = nodes[0] + 1;
      nodes[3] = (j+1) * (x_segments+1) + i;
      nodes[2] = nodes[3] + 1;
    }
  }
}

void create_circle_2d(CArray& coordinates, CTable& connectivity, const Real radius, const Uint segments, const Real start_angle, const Real end_angle)
{
  const Uint dim = 2;
  const Uint nb_nodes = 2;
  const bool closed = std::abs(std::abs(end_angle - start_angle) - 2.0*RealPi) < RealEps;

  coordinates.initialize(dim);
  CArray::ArrayT& coord_array = coordinates.array();
  coord_array.resize(boost::extents[segments + (!closed)][dim]);

  connectivity.initialize(nb_nodes);
  CTable::ArrayT& conn_array = connectivity.array();
  conn_array.resize(boost::extents[segments][nb_nodes]);
  for(Uint u = 0; u != segments; ++u)
  {
    const Real theta = start_angle + (end_angle - start_angle) * (static_cast<Real>(u) / static_cast<Real>(segments));
    CArray::Row coord_row = coord_array[u];

    coord_row[XX] = radius * cos(theta);
    coord_row[YY] = radius * sin(theta);

    CTable::Row nodes = conn_array[u];
    nodes[0] = u;
    nodes[1] = u+1;
  }
  if(closed)
  {
    conn_array[segments-1][1] = 0;
  }
  else
  {
    CArray::Row coord_row = coord_array[segments];
    coord_row[XX] = radius * cos(end_angle);
    coord_row[YY] = radius * sin(end_angle);
  }
}

void create_circle_2d ( CMesh& mesh, const Real radius, const Uint segments, const Real start_angle, const Real end_angle )
{
  CRegion& region = mesh.create_region("region");
  CArray& coordinates = region.create_coordinates(DIM_2D);
  CTable& conn = region.create_elements("Line2DLagrangeP1",coordinates).connectivity_table();
  create_circle_2d(coordinates, conn, radius, segments, start_angle, end_angle);
}

void create_channel_3d(BlockData& blocks, const Real length, const Real half_height, const Real width, const Uint x_segs, const Uint y_segs_half, const Uint z_segs, const Real ratio)
{
  blocks.scaling_factor = 1.;
  
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


