// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Mesh/CGNS/Shared.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace CGNS {

//////////////////////////////////////////////////////////////////////////////

Shared::Shared()
{
  m_supported_element_types.reserve(9);
  m_supported_element_types.push_back("CF.Mesh.LagrangeP1.Line1D");
  m_supported_element_types.push_back("CF.Mesh.LagrangeP1.Line2D");
  m_supported_element_types.push_back("CF.Mesh.LagrangeP1.Line3D");
  m_supported_element_types.push_back("CF.Mesh.LagrangeP1.Triag2D");
  m_supported_element_types.push_back("CF.Mesh.LagrangeP1.Triag3D");
  m_supported_element_types.push_back("CF.Mesh.LagrangeP1.Quad2D");
  m_supported_element_types.push_back("CF.Mesh.LagrangeP1.Quad3D");
  m_supported_element_types.push_back("CF.Mesh.LagrangeP1.Tetra3D");
  m_supported_element_types.push_back("CF.Mesh.LagrangeP1.Hexa3D");

  m_elemtype_CGNS_to_CF[BAR_2  ] = "CF.Mesh.LagrangeP1.Line";
  m_elemtype_CGNS_to_CF[TRI_3  ] = "CF.Mesh.LagrangeP1.Triag";
  m_elemtype_CGNS_to_CF[QUAD_4 ] = "CF.Mesh.LagrangeP1.Quad";
  m_elemtype_CGNS_to_CF[TETRA_4] = "CF.Mesh.LagrangeP1.Tetra";
  m_elemtype_CGNS_to_CF[HEXA_8 ] = "CF.Mesh.LagrangeP1.Hexa";

  m_elemtype_CF_to_CGNS["CF.Mesh.LagrangeP1.Line1D" ] = BAR_2;
  m_elemtype_CF_to_CGNS["CF.Mesh.LagrangeP1.Line2D" ] = BAR_2;
  m_elemtype_CF_to_CGNS["CF.Mesh.LagrangeP1.Line3D" ] = BAR_2;
  m_elemtype_CF_to_CGNS["CF.Mesh.LagrangeP1.Triag2D"] = TRI_3;
  m_elemtype_CF_to_CGNS["CF.Mesh.LagrangeP1.Triag3D"] = TRI_3;
  m_elemtype_CF_to_CGNS["CF.Mesh.LagrangeP1.Quad2D" ] = QUAD_4;
  m_elemtype_CF_to_CGNS["CF.Mesh.LagrangeP1.Quad3D" ] = QUAD_4;
  m_elemtype_CF_to_CGNS["CF.Mesh.LagrangeP1.Tetra3D"] = TETRA_4;
  m_elemtype_CF_to_CGNS["CF.Mesh.LagrangeP1.Hexa3D" ] = HEXA_8;
}

//////////////////////////////////////////////////////////////////////////////

} // CGNS
} // Mesh
} // CF
