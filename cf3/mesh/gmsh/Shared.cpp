// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "mesh/gmsh/Shared.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace gmsh {

//////////////////////////////////////////////////////////////////////////////

const Uint Shared::m_nodes_in_gmsh_elem[nb_gmsh_types] = { 0,  2,  3,  4,  4,  8,  6,  5,  3,  6,  9,
                                                              10, 27, 18, 14,  1,  8, 20, 15, 13,  9,
                                                              10, 12, 15, 15, 21,  4,  5,  6, 20, 35,
                                                              56, 34, 52,  0,  0, 16 };

const Uint Shared::m_gmsh_elem_dim[nb_gmsh_types] = { DIM_0D, DIM_1D, DIM_2D, DIM_2D, DIM_3D, DIM_3D, DIM_3D, DIM_3D, DIM_1D, DIM_2D,
                                                      DIM_2D, DIM_3D, DIM_3D, DIM_3D, DIM_3D, DIM_1D, DIM_2D, DIM_2D, DIM_3D, DIM_3D,
                                                      DIM_2D, DIM_2D, DIM_2D, DIM_2D, DIM_2D, DIM_2D, DIM_1D, DIM_1D, DIM_1D, DIM_3D,
                                                      DIM_3D, DIM_3D, DIM_3D, DIM_3D, DIM_2D, DIM_3D, DIM_2D };

const Uint Shared::m_gmsh_elem_order[nb_gmsh_types] = {  0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
                                                            2, 2, 2, 2, 0, 2, 2, 2, 2, 3,
                                                            3, 4, 4, 5, 5, 3, 4, 5, 3, 4,
                                                            5, 4, 5, 0, 0, 3 };

const std::string Shared::gmsh_elem_geo_name[nb_gmsh_types] = { "Empty", "Line" , "Triag"  , "Quad" , "Tetra"  ,
                                                                "Hexa" , "Prism", "Pyramid", "Line" , "Triag"  ,
                                                                "Quad" , "Tetra", "Hexa"   , "Prism", "Pyramid",
                                                                "Point", "Quad" , "Hexa"   , "Prism", "Pyramid",
                                                                "Triag", "Triag", "Triag"  , "Triag", "Triag"  ,
                                                                "Triag", "Line" , "Line"   , "Line" , "Tetra"  ,
                                                                "Tetra", "Tetra", "Tetra"  , "Tetra", "Polyg"  ,
                                                                "Polyh", "Quad"
                                                               };

const std::string Shared::dim_name[4] = { "0D", "1D", "2D", "3D" };


const std::string Shared::order_name[10] = { "P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7", "P8", "P9" };




//////////////////////////////////////////////////////////////////////////////

Shared::Shared() :
    m_nodes_cf_to_gmsh(nb_gmsh_types),
    m_nodes_gmsh_to_cf(nb_gmsh_types)
{
  m_supported_types.reserve(20);
  m_supported_types.push_back("cf3.mesh.LagrangeP1.Line1D");
  m_supported_types.push_back("cf3.mesh.LagrangeP1.Line2D");
  m_supported_types.push_back("cf3.mesh.LagrangeP2.Line2D");
  m_supported_types.push_back("cf3.mesh.LagrangeP3.Line2D");
  m_supported_types.push_back("cf3.mesh.LagrangeP1.Line3D");
  m_supported_types.push_back("cf3.mesh.LagrangeP1.Quad2D");
  m_supported_types.push_back("cf3.mesh.LagrangeP2.Quad2D");
  m_supported_types.push_back("cf3.mesh.LagrangeP3.Quad2D");
  m_supported_types.push_back("cf3.mesh.LagrangeP1.Quad3D");
  m_supported_types.push_back("cf3.mesh.LagrangeP1.Triag2D");
  m_supported_types.push_back("cf3.mesh.LagrangeP2.Triag2D");
  m_supported_types.push_back("cf3.mesh.LagrangeP3.Triag2D");
  m_supported_types.push_back("cf3.mesh.LagrangeP1.Triag3D");
  m_supported_types.push_back("cf3.mesh.LagrangeP1.Hexa3D");
  m_supported_types.push_back("cf3.mesh.LagrangeP1.Tetra3D");
  m_supported_types.push_back("cf3.mesh.LagrangeP0.Point1D");
  m_supported_types.push_back("cf3.mesh.LagrangeP0.Point2D");
  m_supported_types.push_back("cf3.mesh.LagrangeP0.Point3D");

  m_CFelement_to_GmshElement[GeoShape::LINE ]=P1LINE;
  m_CFelement_to_GmshElement[GeoShape::TRIAG]=P1TRIAG;
  m_CFelement_to_GmshElement[GeoShape::QUAD ]=P1QUAD;
  m_CFelement_to_GmshElement[GeoShape::HEXA ]=P1HEXA;
  m_CFelement_to_GmshElement[GeoShape::TETRA]=P1TETRA;
  m_CFelement_to_GmshElement[GeoShape::POINT]=P0POINT;

  // --------------------------------------------------- NODES

  // P1 line
  m_nodes_cf_to_gmsh[P1LINE].resize(2);
  m_nodes_cf_to_gmsh[P1LINE][0]=0;
  m_nodes_cf_to_gmsh[P1LINE][1]=1;

  m_nodes_gmsh_to_cf[P1LINE].resize(2);
  m_nodes_gmsh_to_cf[P1LINE][0]=0;
  m_nodes_gmsh_to_cf[P1LINE][1]=1;

  // P1 quad
  m_nodes_cf_to_gmsh[P1QUAD].resize(4);
  m_nodes_cf_to_gmsh[P1QUAD][0]=0;
  m_nodes_cf_to_gmsh[P1QUAD][1]=1;
  m_nodes_cf_to_gmsh[P1QUAD][2]=2;
  m_nodes_cf_to_gmsh[P1QUAD][3]=3;

  m_nodes_gmsh_to_cf[P1QUAD].resize(4);
  m_nodes_gmsh_to_cf[P1QUAD][0]=0;
  m_nodes_gmsh_to_cf[P1QUAD][1]=1;
  m_nodes_gmsh_to_cf[P1QUAD][2]=2;
  m_nodes_gmsh_to_cf[P1QUAD][3]=3;

  // P1 triag
  m_nodes_cf_to_gmsh[P1TRIAG].resize(3);
  m_nodes_cf_to_gmsh[P1TRIAG][0]=0;
  m_nodes_cf_to_gmsh[P1TRIAG][1]=1;
  m_nodes_cf_to_gmsh[P1TRIAG][2]=2;

  m_nodes_gmsh_to_cf[P1TRIAG].resize(3);
  m_nodes_gmsh_to_cf[P1TRIAG][0]=0;
  m_nodes_gmsh_to_cf[P1TRIAG][1]=1;
  m_nodes_gmsh_to_cf[P1TRIAG][2]=2;


  // P1 tetra
  m_nodes_cf_to_gmsh[P1TETRA].resize(4);
  m_nodes_cf_to_gmsh[P1TETRA][0]=0;
  m_nodes_cf_to_gmsh[P1TETRA][1]=1;
  m_nodes_cf_to_gmsh[P1TETRA][2]=2;
  m_nodes_cf_to_gmsh[P1TETRA][3]=3;

  m_nodes_gmsh_to_cf[P1TETRA].resize(4);
  m_nodes_gmsh_to_cf[P1TETRA][0]=0;
  m_nodes_gmsh_to_cf[P1TETRA][1]=1;
  m_nodes_gmsh_to_cf[P1TETRA][2]=2;
  m_nodes_gmsh_to_cf[P1TETRA][3]=3;


  // P1 hexa
  m_nodes_cf_to_gmsh[P1HEXA].resize(8);
  m_nodes_cf_to_gmsh[P1HEXA][0]=0;
  m_nodes_cf_to_gmsh[P1HEXA][1]=1;
  m_nodes_cf_to_gmsh[P1HEXA][2]=2;
  m_nodes_cf_to_gmsh[P1HEXA][3]=3;
  m_nodes_cf_to_gmsh[P1HEXA][4]=4;
  m_nodes_cf_to_gmsh[P1HEXA][5]=5;
  m_nodes_cf_to_gmsh[P1HEXA][6]=6;
  m_nodes_cf_to_gmsh[P1HEXA][7]=7;

  m_nodes_gmsh_to_cf[P1HEXA].resize(8);
  m_nodes_gmsh_to_cf[P1HEXA][0]=0;
  m_nodes_gmsh_to_cf[P1HEXA][1]=1;
  m_nodes_gmsh_to_cf[P1HEXA][2]=2;
  m_nodes_gmsh_to_cf[P1HEXA][3]=3;
  m_nodes_gmsh_to_cf[P1HEXA][4]=4;
  m_nodes_gmsh_to_cf[P1HEXA][5]=5;
  m_nodes_gmsh_to_cf[P1HEXA][6]=6;
  m_nodes_gmsh_to_cf[P1HEXA][7]=7;


  // P2 line
  m_nodes_cf_to_gmsh[P2LINE].resize(3);
  m_nodes_cf_to_gmsh[P2LINE][0]=0;
  m_nodes_cf_to_gmsh[P2LINE][1]=1;
  m_nodes_cf_to_gmsh[P2LINE][2]=2;

  m_nodes_gmsh_to_cf[P2LINE].resize(3);
  m_nodes_gmsh_to_cf[P2LINE][0]=0;
  m_nodes_gmsh_to_cf[P2LINE][1]=1;
  m_nodes_gmsh_to_cf[P2LINE][2]=2;

  // P2 quad
  m_nodes_cf_to_gmsh[P2QUAD].resize(9);
  m_nodes_cf_to_gmsh[P2QUAD][0]=0;
  m_nodes_cf_to_gmsh[P2QUAD][1]=1;
  m_nodes_cf_to_gmsh[P2QUAD][2]=2;
  m_nodes_cf_to_gmsh[P2QUAD][3]=3;
  m_nodes_cf_to_gmsh[P2QUAD][4]=4;
  m_nodes_cf_to_gmsh[P2QUAD][5]=5;
  m_nodes_cf_to_gmsh[P2QUAD][6]=6;
  m_nodes_cf_to_gmsh[P2QUAD][7]=7;
  m_nodes_cf_to_gmsh[P2QUAD][8]=8;

  m_nodes_gmsh_to_cf[P2QUAD].resize(9);
  m_nodes_gmsh_to_cf[P2QUAD][0]=0;
  m_nodes_gmsh_to_cf[P2QUAD][1]=1;
  m_nodes_gmsh_to_cf[P2QUAD][2]=2;
  m_nodes_gmsh_to_cf[P2QUAD][3]=3;
  m_nodes_gmsh_to_cf[P2QUAD][4]=4;
  m_nodes_gmsh_to_cf[P2QUAD][5]=5;
  m_nodes_gmsh_to_cf[P2QUAD][6]=6;
  m_nodes_gmsh_to_cf[P2QUAD][7]=7;
  m_nodes_gmsh_to_cf[P2QUAD][8]=8;


  // P2 triag
  m_nodes_cf_to_gmsh[P2TRIAG].resize(6);
  m_nodes_cf_to_gmsh[P2TRIAG][0]=0;
  m_nodes_cf_to_gmsh[P2TRIAG][1]=1;
  m_nodes_cf_to_gmsh[P2TRIAG][2]=2;
  m_nodes_cf_to_gmsh[P2TRIAG][3]=3;
  m_nodes_cf_to_gmsh[P2TRIAG][4]=4;
  m_nodes_cf_to_gmsh[P2TRIAG][5]=5;

  m_nodes_gmsh_to_cf[P2TRIAG].resize(6);
  m_nodes_gmsh_to_cf[P2TRIAG][0]=0;
  m_nodes_gmsh_to_cf[P2TRIAG][1]=1;
  m_nodes_gmsh_to_cf[P2TRIAG][2]=2;
  m_nodes_gmsh_to_cf[P2TRIAG][3]=3;
  m_nodes_gmsh_to_cf[P2TRIAG][4]=4;
  m_nodes_gmsh_to_cf[P2TRIAG][5]=5;


  // P2 tetra
  m_nodes_cf_to_gmsh[P2TETRA].resize(10);
  m_nodes_cf_to_gmsh[P2TETRA][0]=0;
  m_nodes_cf_to_gmsh[P2TETRA][1]=1;
  m_nodes_cf_to_gmsh[P2TETRA][2]=2;
  m_nodes_cf_to_gmsh[P2TETRA][3]=3;
  m_nodes_cf_to_gmsh[P2TETRA][4]=4;
  m_nodes_cf_to_gmsh[P2TETRA][5]=5;
  m_nodes_cf_to_gmsh[P2TETRA][6]=6;
  m_nodes_cf_to_gmsh[P2TETRA][7]=7;
  m_nodes_cf_to_gmsh[P2TETRA][8]=8;
  m_nodes_cf_to_gmsh[P2TETRA][9]=9;

  m_nodes_gmsh_to_cf[P2TETRA].resize(10);
  m_nodes_gmsh_to_cf[P2TETRA][0]=0;
  m_nodes_gmsh_to_cf[P2TETRA][1]=1;
  m_nodes_gmsh_to_cf[P2TETRA][2]=2;
  m_nodes_gmsh_to_cf[P2TETRA][3]=3;
  m_nodes_gmsh_to_cf[P2TETRA][4]=4;
  m_nodes_gmsh_to_cf[P2TETRA][5]=5;
  m_nodes_gmsh_to_cf[P2TETRA][6]=6;
  m_nodes_gmsh_to_cf[P2TETRA][7]=7;
  m_nodes_gmsh_to_cf[P2TETRA][8]=8;
  m_nodes_gmsh_to_cf[P2TETRA][9]=9;


  // P2 hexa
  m_nodes_cf_to_gmsh[P2HEXA].resize(27);
  m_nodes_cf_to_gmsh[P2HEXA][0]=0;
  m_nodes_cf_to_gmsh[P2HEXA][1]=1;
  m_nodes_cf_to_gmsh[P2HEXA][2]=2;
  m_nodes_cf_to_gmsh[P2HEXA][3]=3;
  m_nodes_cf_to_gmsh[P2HEXA][4]=4;
  m_nodes_cf_to_gmsh[P2HEXA][5]=5;
  m_nodes_cf_to_gmsh[P2HEXA][6]=6;
  m_nodes_cf_to_gmsh[P2HEXA][7]=7;
  m_nodes_cf_to_gmsh[P2HEXA][8]=8;
  m_nodes_cf_to_gmsh[P2HEXA][9]=9;
  m_nodes_cf_to_gmsh[P2HEXA][10]=10;
  m_nodes_cf_to_gmsh[P2HEXA][11]=11;
  m_nodes_cf_to_gmsh[P2HEXA][12]=12;
  m_nodes_cf_to_gmsh[P2HEXA][13]=13;
  m_nodes_cf_to_gmsh[P2HEXA][14]=14;
  m_nodes_cf_to_gmsh[P2HEXA][15]=15;
  m_nodes_cf_to_gmsh[P2HEXA][16]=16;
  m_nodes_cf_to_gmsh[P2HEXA][17]=17;
  m_nodes_cf_to_gmsh[P2HEXA][18]=18;
  m_nodes_cf_to_gmsh[P2HEXA][19]=19;
  m_nodes_cf_to_gmsh[P2HEXA][20]=20;
  m_nodes_cf_to_gmsh[P2HEXA][21]=21;
  m_nodes_cf_to_gmsh[P2HEXA][22]=22;
  m_nodes_cf_to_gmsh[P2HEXA][23]=23;
  m_nodes_cf_to_gmsh[P2HEXA][24]=24;
  m_nodes_cf_to_gmsh[P2HEXA][25]=25;
  m_nodes_cf_to_gmsh[P2HEXA][26]=26;

  m_nodes_gmsh_to_cf[P2HEXA].resize(27);
  m_nodes_gmsh_to_cf[P2HEXA][0]=0;
  m_nodes_gmsh_to_cf[P2HEXA][1]=1;
  m_nodes_gmsh_to_cf[P2HEXA][2]=2;
  m_nodes_gmsh_to_cf[P2HEXA][3]=3;
  m_nodes_gmsh_to_cf[P2HEXA][4]=4;
  m_nodes_gmsh_to_cf[P2HEXA][5]=5;
  m_nodes_gmsh_to_cf[P2HEXA][6]=6;
  m_nodes_gmsh_to_cf[P2HEXA][7]=7;
  m_nodes_gmsh_to_cf[P2HEXA][8]=8;
  m_nodes_gmsh_to_cf[P2HEXA][9]=9;
  m_nodes_gmsh_to_cf[P2HEXA][10]=10;
  m_nodes_gmsh_to_cf[P2HEXA][11]=11;
  m_nodes_gmsh_to_cf[P2HEXA][12]=12;
  m_nodes_gmsh_to_cf[P2HEXA][13]=13;
  m_nodes_gmsh_to_cf[P2HEXA][14]=14;
  m_nodes_gmsh_to_cf[P2HEXA][15]=15;
  m_nodes_gmsh_to_cf[P2HEXA][16]=16;
  m_nodes_gmsh_to_cf[P2HEXA][17]=17;
  m_nodes_gmsh_to_cf[P2HEXA][18]=18;
  m_nodes_gmsh_to_cf[P2HEXA][19]=19;
  m_nodes_gmsh_to_cf[P2HEXA][20]=20;
  m_nodes_gmsh_to_cf[P2HEXA][21]=21;
  m_nodes_gmsh_to_cf[P2HEXA][22]=22;
  m_nodes_gmsh_to_cf[P2HEXA][23]=23;
  m_nodes_gmsh_to_cf[P2HEXA][24]=24;
  m_nodes_gmsh_to_cf[P2HEXA][25]=25;
  m_nodes_gmsh_to_cf[P2HEXA][26]=26;

  //Point
  m_nodes_gmsh_to_cf[P0POINT].resize(1);
  m_nodes_gmsh_to_cf[P0POINT][0]=0;

  //P3 triag
  m_nodes_cf_to_gmsh[P3TRIAG].resize(10);
  m_nodes_cf_to_gmsh[P3TRIAG][0] = 0;
  m_nodes_cf_to_gmsh[P3TRIAG][1] = 1;
  m_nodes_cf_to_gmsh[P3TRIAG][2] = 2;
  m_nodes_cf_to_gmsh[P3TRIAG][3] = 3;
  m_nodes_cf_to_gmsh[P3TRIAG][4] = 4;
  m_nodes_cf_to_gmsh[P3TRIAG][5] = 5;
  m_nodes_cf_to_gmsh[P3TRIAG][6] = 6;
  m_nodes_cf_to_gmsh[P3TRIAG][7] = 7;
  m_nodes_cf_to_gmsh[P3TRIAG][8] = 8;
  m_nodes_cf_to_gmsh[P3TRIAG][9] = 9;


  m_nodes_gmsh_to_cf[P3TRIAG].resize(10);
  m_nodes_gmsh_to_cf[P3TRIAG][0] = 0;
  m_nodes_gmsh_to_cf[P3TRIAG][1] = 1;
  m_nodes_gmsh_to_cf[P3TRIAG][2] = 2;
  m_nodes_gmsh_to_cf[P3TRIAG][3] = 3;
  m_nodes_gmsh_to_cf[P3TRIAG][4] = 4;
  m_nodes_gmsh_to_cf[P3TRIAG][5] = 5;
  m_nodes_gmsh_to_cf[P3TRIAG][6] = 6;
  m_nodes_gmsh_to_cf[P3TRIAG][7] = 7;
  m_nodes_gmsh_to_cf[P3TRIAG][8] = 8;
  m_nodes_gmsh_to_cf[P3TRIAG][9] = 9;

  //P3 line
  m_nodes_cf_to_gmsh[P3LINE].resize(4);
  m_nodes_cf_to_gmsh[P3LINE][0] = 0;
  m_nodes_cf_to_gmsh[P3LINE][1] = 1;
  m_nodes_cf_to_gmsh[P3LINE][2] = 2;
  m_nodes_cf_to_gmsh[P3LINE][3] = 3;

  m_nodes_gmsh_to_cf[P3LINE].resize(4);
  m_nodes_gmsh_to_cf[P3LINE][0] = 0;
  m_nodes_gmsh_to_cf[P3LINE][1] = 1;
  m_nodes_gmsh_to_cf[P3LINE][2] = 2;
  m_nodes_gmsh_to_cf[P3LINE][3] = 3;

  //P3 quad (16 nodes)
  m_nodes_cf_to_gmsh[P3QUAD].resize(16);
  m_nodes_cf_to_gmsh[P3QUAD][0]  = 0;
  m_nodes_cf_to_gmsh[P3QUAD][1]  = 1;
  m_nodes_cf_to_gmsh[P3QUAD][2]  = 2;
  m_nodes_cf_to_gmsh[P3QUAD][3]  = 3;
  m_nodes_cf_to_gmsh[P3QUAD][4]  = 4;
  m_nodes_cf_to_gmsh[P3QUAD][5]  = 5;
  m_nodes_cf_to_gmsh[P3QUAD][6]  = 6;
  m_nodes_cf_to_gmsh[P3QUAD][7]  = 7;
  m_nodes_cf_to_gmsh[P3QUAD][8]  = 8;
  m_nodes_cf_to_gmsh[P3QUAD][9]  = 9;
  m_nodes_cf_to_gmsh[P3QUAD][10] = 10;
  m_nodes_cf_to_gmsh[P3QUAD][11] = 11;
  m_nodes_cf_to_gmsh[P3QUAD][12] = 12;
  m_nodes_cf_to_gmsh[P3QUAD][13] = 13;
  m_nodes_cf_to_gmsh[P3QUAD][14] = 14;
  m_nodes_cf_to_gmsh[P3QUAD][15] = 15;

  m_nodes_gmsh_to_cf[P3QUAD].resize(16);
  m_nodes_gmsh_to_cf[P3QUAD][0]  = 0;
  m_nodes_gmsh_to_cf[P3QUAD][1]  = 1;
  m_nodes_gmsh_to_cf[P3QUAD][2]  = 2;
  m_nodes_gmsh_to_cf[P3QUAD][3]  = 3;
  m_nodes_gmsh_to_cf[P3QUAD][4]  = 4;
  m_nodes_gmsh_to_cf[P3QUAD][5]  = 5;
  m_nodes_gmsh_to_cf[P3QUAD][6]  = 6;
  m_nodes_gmsh_to_cf[P3QUAD][7]  = 7;
  m_nodes_gmsh_to_cf[P3QUAD][8]  = 8;
  m_nodes_gmsh_to_cf[P3QUAD][9]  = 9;
  m_nodes_gmsh_to_cf[P3QUAD][10] = 10;
  m_nodes_gmsh_to_cf[P3QUAD][11] = 11;
  m_nodes_gmsh_to_cf[P3QUAD][12] = 12;
  m_nodes_gmsh_to_cf[P3QUAD][13] = 13;
  m_nodes_gmsh_to_cf[P3QUAD][14] = 14;
  m_nodes_gmsh_to_cf[P3QUAD][15] = 15;

}


//////////////////////////////////////////////////////////////////////////////

std::string Shared::gmsh_name_to_cf_name(const Uint dim, const Uint gmsh_type)
{
  //Compose the name of the form   "cf3.mesh.LagrangeP1.Line1D"
  const Uint order = m_gmsh_elem_order[gmsh_type];
  std::string name = "cf3.mesh.Lagrange"+order_name[order]+"." + gmsh_elem_geo_name[gmsh_type] + dim_name[dim];
  return name;
}

//////////////////////////////////////////////////////////////////////////////

} // gmsh
} // mesh
} // cf3
