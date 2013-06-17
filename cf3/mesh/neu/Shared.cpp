// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "mesh/neu/Shared.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace neu {
  
//////////////////////////////////////////////////////////////////////////////

Shared::Shared() :
    m_faces_cf_to_neu(10),
    m_faces_neu_to_cf(10),
    m_nodes_cf_to_neu(10),
    m_nodes_neu_to_cf(10)
{
  m_supported_types.reserve(9);
  m_supported_types.push_back("cf3.mesh.LagrangeP1.Line1D");
  m_supported_types.push_back("cf3.mesh.LagrangeP1.Line2D");
  m_supported_types.push_back("cf3.mesh.LagrangeP1.Line3D");
  m_supported_types.push_back("cf3.mesh.LagrangeP1.Quad2D");
  m_supported_types.push_back("cf3.mesh.LagrangeP1.Quad3D");
  m_supported_types.push_back("cf3.mesh.LagrangeP1.Triag2D");
  m_supported_types.push_back("cf3.mesh.LagrangeP1.Triag3D");
  m_supported_types.push_back("cf3.mesh.LagrangeP1.Hexa3D");
  m_supported_types.push_back("cf3.mesh.LagrangeP1.Tetra3D");
  m_supported_types.push_back("cf3.mesh.LagrangeP1.Prism3D");

  m_CFelement_to_neuElement[GeoShape::LINE ]=LINE;
  m_CFelement_to_neuElement[GeoShape::QUAD ]=QUAD;
  m_CFelement_to_neuElement[GeoShape::TRIAG]=TRIAG;
  m_CFelement_to_neuElement[GeoShape::HEXA ]=HEXA;
  m_CFelement_to_neuElement[GeoShape::TETRA]=TETRA;
  m_CFelement_to_neuElement[GeoShape::PRISM]=PRISM;

  m_supported_neu_types.insert(LINE);
  m_supported_neu_types.insert(QUAD);
  m_supported_neu_types.insert(TRIAG);
  m_supported_neu_types.insert(HEXA);
  m_supported_neu_types.insert(TETRA);
  m_supported_neu_types.insert(PRISM);

  // ------------------------------------------------------- FACES
  // line
  m_faces_cf_to_neu[LINE].resize(2);
  m_faces_cf_to_neu[LINE][0]=1;
  m_faces_cf_to_neu[LINE][1]=2;

  m_faces_neu_to_cf[LINE].resize(3);
  m_faces_neu_to_cf[LINE][1]=0;
  m_faces_neu_to_cf[LINE][2]=1;

  // quad
  m_faces_cf_to_neu[QUAD].resize(4);
  m_faces_cf_to_neu[QUAD][0]=1;
  m_faces_cf_to_neu[QUAD][1]=2;
  m_faces_cf_to_neu[QUAD][2]=3;
  m_faces_cf_to_neu[QUAD][3]=4;

  m_faces_neu_to_cf[QUAD].resize(5);
  m_faces_neu_to_cf[QUAD][1]=0;
  m_faces_neu_to_cf[QUAD][2]=1;
  m_faces_neu_to_cf[QUAD][3]=2;
  m_faces_neu_to_cf[QUAD][4]=3;

  // triag
  m_faces_cf_to_neu[TRIAG].resize(3);
  m_faces_cf_to_neu[TRIAG][0]=1;
  m_faces_cf_to_neu[TRIAG][1]=2;
  m_faces_cf_to_neu[TRIAG][2]=3;

  m_faces_neu_to_cf[TRIAG].resize(4);
  m_faces_neu_to_cf[TRIAG][1]=0;
  m_faces_neu_to_cf[TRIAG][2]=1;
  m_faces_neu_to_cf[TRIAG][3]=2;

  // hexa
  m_faces_cf_to_neu[HEXA].resize(6);
  m_faces_cf_to_neu[HEXA][0]=1;
  m_faces_cf_to_neu[HEXA][1]=3;
  m_faces_cf_to_neu[HEXA][2]=6;
  m_faces_cf_to_neu[HEXA][3]=2;
  m_faces_cf_to_neu[HEXA][4]=5;
  m_faces_cf_to_neu[HEXA][5]=4;

  m_faces_neu_to_cf[HEXA].resize(7);
  m_faces_neu_to_cf[HEXA][1]=0;
  m_faces_neu_to_cf[HEXA][2]=3;
  m_faces_neu_to_cf[HEXA][3]=1;
  m_faces_neu_to_cf[HEXA][4]=5;
  m_faces_neu_to_cf[HEXA][5]=4;
  m_faces_neu_to_cf[HEXA][6]=2;

  // tetra
  m_faces_cf_to_neu[TETRA].resize(4);
  m_faces_cf_to_neu[TETRA][0]=1;
  m_faces_cf_to_neu[TETRA][1]=2;
  m_faces_cf_to_neu[TETRA][2]=3;
  m_faces_cf_to_neu[TETRA][3]=4;

  m_faces_neu_to_cf[TETRA].resize(5);
  m_faces_neu_to_cf[TETRA][1]=0;
  m_faces_neu_to_cf[TETRA][2]=1;
  m_faces_neu_to_cf[TETRA][3]=2;
  m_faces_neu_to_cf[TETRA][4]=3;
  
  // prism
  m_faces_cf_to_neu[PRISM].resize(5);
  m_faces_cf_to_neu[PRISM][0]=4;
  m_faces_cf_to_neu[PRISM][1]=5;
  m_faces_cf_to_neu[PRISM][2]=3;
  m_faces_cf_to_neu[PRISM][3]=1;
  m_faces_cf_to_neu[PRISM][4]=2;
  
  m_faces_neu_to_cf[PRISM].resize(6);
  m_faces_neu_to_cf[PRISM][1]=3;
  m_faces_neu_to_cf[PRISM][2]=4;
  m_faces_neu_to_cf[PRISM][3]=2;
  m_faces_neu_to_cf[PRISM][4]=0;
  m_faces_neu_to_cf[PRISM][5]=1;


  // --------------------------------------------------- NODES

  // line
  m_nodes_cf_to_neu[LINE].resize(2);
  m_nodes_cf_to_neu[LINE][0]=0;
  m_nodes_cf_to_neu[LINE][1]=1;

  m_nodes_neu_to_cf[LINE].resize(2);
  m_nodes_neu_to_cf[LINE][0]=0;
  m_nodes_neu_to_cf[LINE][1]=1;

  // quad
  m_nodes_cf_to_neu[QUAD].resize(4);
  m_nodes_cf_to_neu[QUAD][0]=0;
  m_nodes_cf_to_neu[QUAD][1]=1;
  m_nodes_cf_to_neu[QUAD][2]=2;
  m_nodes_cf_to_neu[QUAD][3]=3;

  m_nodes_neu_to_cf[QUAD].resize(4);
  m_nodes_neu_to_cf[QUAD][0]=0;
  m_nodes_neu_to_cf[QUAD][1]=1;
  m_nodes_neu_to_cf[QUAD][2]=2;
  m_nodes_neu_to_cf[QUAD][3]=3;

  // triag
  m_nodes_cf_to_neu[TRIAG].resize(3);
  m_nodes_cf_to_neu[TRIAG][0]=0;
  m_nodes_cf_to_neu[TRIAG][1]=1;
  m_nodes_cf_to_neu[TRIAG][2]=2;

  m_nodes_neu_to_cf[TRIAG].resize(3);
  m_nodes_neu_to_cf[TRIAG][0]=0;
  m_nodes_neu_to_cf[TRIAG][1]=1;
  m_nodes_neu_to_cf[TRIAG][2]=2;


  // tetra
  m_nodes_cf_to_neu[TETRA].resize(4);
  m_nodes_cf_to_neu[TETRA][0]=0;
  m_nodes_cf_to_neu[TETRA][1]=1;
  m_nodes_cf_to_neu[TETRA][2]=2;
  m_nodes_cf_to_neu[TETRA][3]=3;

  m_nodes_neu_to_cf[TETRA].resize(4);
  m_nodes_neu_to_cf[TETRA][0]=0;
  m_nodes_neu_to_cf[TETRA][1]=1;
  m_nodes_neu_to_cf[TETRA][2]=2;
  m_nodes_neu_to_cf[TETRA][3]=3;


  // hexa
  m_nodes_cf_to_neu[HEXA].resize(8);
  m_nodes_cf_to_neu[HEXA][0]=4;
  m_nodes_cf_to_neu[HEXA][1]=5;
  m_nodes_cf_to_neu[HEXA][2]=1;
  m_nodes_cf_to_neu[HEXA][3]=0;
  m_nodes_cf_to_neu[HEXA][4]=6;
  m_nodes_cf_to_neu[HEXA][5]=7;
  m_nodes_cf_to_neu[HEXA][6]=3;
  m_nodes_cf_to_neu[HEXA][7]=2;

  m_nodes_neu_to_cf[HEXA].resize(8);
  m_nodes_neu_to_cf[HEXA][0]=3;
  m_nodes_neu_to_cf[HEXA][1]=2;
  m_nodes_neu_to_cf[HEXA][2]=7;
  m_nodes_neu_to_cf[HEXA][3]=6;
  m_nodes_neu_to_cf[HEXA][4]=0;
  m_nodes_neu_to_cf[HEXA][5]=1;
  m_nodes_neu_to_cf[HEXA][6]=4;
  m_nodes_neu_to_cf[HEXA][7]=5;
  
  // prism
  m_nodes_cf_to_neu[PRISM].resize(6);
  m_nodes_cf_to_neu[PRISM][0]=0;
  m_nodes_cf_to_neu[PRISM][1]=1;
  m_nodes_cf_to_neu[PRISM][2]=2;
  m_nodes_cf_to_neu[PRISM][3]=3;
  m_nodes_cf_to_neu[PRISM][4]=4;
  m_nodes_cf_to_neu[PRISM][5]=5;
  
  m_nodes_neu_to_cf[PRISM].resize(6);
  m_nodes_neu_to_cf[PRISM][0]=0;
  m_nodes_neu_to_cf[PRISM][1]=1;
  m_nodes_neu_to_cf[PRISM][2]=2;
  m_nodes_neu_to_cf[PRISM][3]=3;
  m_nodes_neu_to_cf[PRISM][4]=4;
  m_nodes_neu_to_cf[PRISM][5]=5;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

} // neu
} // mesh
} // cf3
