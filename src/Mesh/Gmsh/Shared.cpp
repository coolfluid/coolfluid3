// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Mesh/Gmsh/Shared.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Gmsh {
  
//////////////////////////////////////////////////////////////////////////////

Shared::Shared() :
    //m_faces_cf_to_gmsh(13),
    //m_faces_gmsh_to_cf(13),
    m_nodes_cf_to_gmsh(13),
    m_nodes_gmsh_to_cf(13)
{
  m_supported_types.reserve(10);
  m_supported_types.push_back("LineLagrangeP1");
  m_supported_types.push_back("TriagLagrangeP1");
  m_supported_types.push_back("QuadLagrangeP1");
  m_supported_types.push_back("TetraLagrangeP1");
  m_supported_types.push_back("HexaLagrangeP1");
  m_supported_types.push_back("LineLagrangeP2");
  m_supported_types.push_back("TriagLagrangeP2");
  m_supported_types.push_back("QuadLagrangeP2");
  m_supported_types.push_back("TetraLagrangeP2");
  m_supported_types.push_back("HexaLagrangeP2");

  enum GmshElement {P1LINE=1,P1TRIAG=2,P1QUAD=3, P1TETRA=4, P1HEXA=5,
                    P2LINE=8,P2TRIAG=6,P21UAD=10,P2TETRA=11,P2HEXA= 12};


  m_CFelement_to_GmshElement[GeoShape::LINE ]=P1LINE;
  m_CFelement_to_GmshElement[GeoShape::TRIAG]=P1TRIAG;
  m_CFelement_to_GmshElement[GeoShape::QUAD ]=P1QUAD;
  m_CFelement_to_GmshElement[GeoShape::HEXA ]=P1HEXA;
  m_CFelement_to_GmshElement[GeoShape::TETRA]=P1TETRA;

  /*
  // ------------------------------------------------------- FACES
  // line
  m_faces_cf_to_gmsh[P1LINE].resize(2);
  m_faces_cf_to_gmsh[P1LINE][0]=0;
  m_faces_cf_to_gmsh[P1LINE][1]=1;

  m_faces_gmsh_to_cf[P1LINE].resize(2);
  m_faces_gmsh_to_cf[P1LINE][1]=0;
  m_faces_gmsh_to_cf[P1LINE][2]=1;

  // triag
  m_faces_cf_to_gmsh[P1TRIAG].resize(3);
  m_faces_cf_to_gmsh[P1TRIAG][0]=0;
  m_faces_cf_to_gmsh[P1TRIAG][1]=1;
  m_faces_cf_to_gmsh[P1TRIAG][2]=2;

  m_faces_gmsh_to_cf[P1TRIAG].resize(3);
  m_faces_gmsh_to_cf[P1TRIAG][0]=0;
  m_faces_gmsh_to_cf[P1TRIAG][1]=1;
  m_faces_gmsh_to_cf[P1TRIAG][2]=2;

  // quad
  m_faces_cf_to_gmsh[P1QUAD].resize(4);
  m_faces_cf_to_gmsh[P1QUAD][0]=0;
  m_faces_cf_to_gmsh[P1QUAD][1]=1;
  m_faces_cf_to_gmsh[P1QUAD][2]=2;
  m_faces_cf_to_gmsh[P1QUAD][3]=3;

  m_faces_gmsh_to_cf[P1QUAD].resize(4);
  m_faces_gmsh_to_cf[P1QUAD][0]=0;
  m_faces_gmsh_to_cf[P1QUAD][1]=1;
  m_faces_gmsh_to_cf[P1QUAD][2]=2;
  m_faces_gmsh_to_cf[P1QUAD][4]=3;

  // tetra
  m_faces_cf_to_gmsh[P1TETRA].resize(4);
  m_faces_cf_to_gmsh[P1TETRA][0]=0;
  m_faces_cf_to_gmsh[P1TETRA][1]=1;
  m_faces_cf_to_gmsh[P1TETRA][2]=2;
  m_faces_cf_to_gmsh[P1TETRA][3]=3;

  m_faces_gmsh_to_cf[P1TETRA].resize(4);
  m_faces_gmsh_to_cf[P1TETRA][0]=0;
  m_faces_gmsh_to_cf[P1TETRA][1]=1;
  m_faces_gmsh_to_cf[P1TETRA][2]=2;
  m_faces_gmsh_to_cf[P1TETRA][3]=3;


  // hexa
  m_faces_cf_to_gmsh[P1HEXA].resize(6);
  m_faces_cf_to_gmsh[P1HEXA][0]=0;
  m_faces_cf_to_gmsh[P1HEXA][1]=1;
  m_faces_cf_to_gmsh[P1HEXA][2]=2;
  m_faces_cf_to_gmsh[P1HEXA][3]=3;
  m_faces_cf_to_gmsh[P1HEXA][4]=4;
  m_faces_cf_to_gmsh[P1HEXA][5]=5;

  m_faces_gmsh_to_cf[P1HEXA].resize(6);
  m_faces_gmsh_to_cf[P1HEXA][0]=0;
  m_faces_gmsh_to_cf[P1HEXA][1]=1;
  m_faces_gmsh_to_cf[P1HEXA][2]=2;
  m_faces_gmsh_to_cf[P1HEXA][3]=3;
  m_faces_gmsh_to_cf[P1HEXA][4]=4;
  m_faces_gmsh_to_cf[P1HEXA][5]=5;
  */

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
  m_nodes_cf_to_gmsh[P1HEXA][0]=4;
  m_nodes_cf_to_gmsh[P1HEXA][1]=5;
  m_nodes_cf_to_gmsh[P1HEXA][2]=1;
  m_nodes_cf_to_gmsh[P1HEXA][3]=0;
  m_nodes_cf_to_gmsh[P1HEXA][4]=6;
  m_nodes_cf_to_gmsh[P1HEXA][5]=7;
  m_nodes_cf_to_gmsh[P1HEXA][6]=3;
  m_nodes_cf_to_gmsh[P1HEXA][7]=2;

  m_nodes_gmsh_to_cf[P1HEXA].resize(8);
  m_nodes_gmsh_to_cf[P1HEXA][0]=3;
  m_nodes_gmsh_to_cf[P1HEXA][1]=2;
  m_nodes_gmsh_to_cf[P1HEXA][2]=7;
  m_nodes_gmsh_to_cf[P1HEXA][3]=6;
  m_nodes_gmsh_to_cf[P1HEXA][4]=0;
  m_nodes_gmsh_to_cf[P1HEXA][5]=1;
  m_nodes_gmsh_to_cf[P1HEXA][6]=4;
  m_nodes_gmsh_to_cf[P1HEXA][7]=5;

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
  m_nodes_cf_to_gmsh[P2HEXA].resize(26);
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

  m_nodes_gmsh_to_cf[P2HEXA].resize(26);
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

}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

} // gmsh
} // Mesh
} // CF
