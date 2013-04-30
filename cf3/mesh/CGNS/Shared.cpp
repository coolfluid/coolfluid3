// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "mesh/CGNS/Shared.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace CGNS {

//////////////////////////////////////////////////////////////////////////////

Shared::Shared()
{
  m_supported_element_types.reserve(9);
  m_supported_element_types.push_back("cf3.mesh.LagrangeP1.Line1D");
  m_supported_element_types.push_back("cf3.mesh.LagrangeP1.Line2D");
  m_supported_element_types.push_back("cf3.mesh.LagrangeP1.Line3D");
  m_supported_element_types.push_back("cf3.mesh.LagrangeP1.Triag2D");
  m_supported_element_types.push_back("cf3.mesh.LagrangeP1.Triag3D");
  m_supported_element_types.push_back("cf3.mesh.LagrangeP1.Quad2D");
  m_supported_element_types.push_back("cf3.mesh.LagrangeP1.Quad3D");
  m_supported_element_types.push_back("cf3.mesh.LagrangeP1.Tetra3D");
  m_supported_element_types.push_back("cf3.mesh.LagrangeP1.Hexa3D");

  m_elemtype_CGNS_to_CF[CGNS_ENUMV( BAR_2 ) ] = "cf3.mesh.LagrangeP1.Line";
  m_elemtype_CGNS_to_CF[CGNS_ENUMV( TRI_3 ) ] = "cf3.mesh.LagrangeP1.Triag";
  m_elemtype_CGNS_to_CF[CGNS_ENUMV( QUAD_4) ] = "cf3.mesh.LagrangeP1.Quad";
  m_elemtype_CGNS_to_CF[CGNS_ENUMV( TETRA_4)] = "cf3.mesh.LagrangeP1.Tetra";
  m_elemtype_CGNS_to_CF[CGNS_ENUMV( HEXA_8 )] = "cf3.mesh.LagrangeP1.Hexa";

  m_elemtype_CF3_to_CGNS["cf3.mesh.LagrangeP1.Line1D" ] = CGNS_ENUMV( BAR_2 );
  m_elemtype_CF3_to_CGNS["cf3.mesh.LagrangeP1.Line2D" ] = CGNS_ENUMV( BAR_2 );
  m_elemtype_CF3_to_CGNS["cf3.mesh.LagrangeP1.Line3D" ] = CGNS_ENUMV( BAR_2 );
  m_elemtype_CF3_to_CGNS["cf3.mesh.LagrangeP1.Triag2D"] = CGNS_ENUMV( TRI_3 );
  m_elemtype_CF3_to_CGNS["cf3.mesh.LagrangeP1.Triag3D"] = CGNS_ENUMV( TRI_3 );
  m_elemtype_CF3_to_CGNS["cf3.mesh.LagrangeP1.Quad2D" ] = CGNS_ENUMV( QUAD_4 );
  m_elemtype_CF3_to_CGNS["cf3.mesh.LagrangeP1.Quad3D" ] = CGNS_ENUMV( QUAD_4 );
  m_elemtype_CF3_to_CGNS["cf3.mesh.LagrangeP1.Tetra3D"] = CGNS_ENUMV( TETRA_4 );
  m_elemtype_CF3_to_CGNS["cf3.mesh.LagrangeP1.Hexa3D" ] = CGNS_ENUMV( HEXA_8 );
}

//////////////////////////////////////////////////////////////////////////////

} // CGNS
} // mesh
} // cf3
