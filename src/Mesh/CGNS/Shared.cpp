// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
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
  m_supported_element_types.push_back("CF.Mesh.SF.Line1DLagrangeP1");
  m_supported_element_types.push_back("CF.Mesh.SF.Line2DLagrangeP1");
  m_supported_element_types.push_back("CF.Mesh.SF.Line3DLagrangeP1");
  m_supported_element_types.push_back("CF.Mesh.SF.Triag2DLagrangeP1");
  m_supported_element_types.push_back("CF.Mesh.SF.Triag3DLagrangeP1");
  m_supported_element_types.push_back("CF.Mesh.SF.Quad2DLagrangeP1");
  m_supported_element_types.push_back("CF.Mesh.SF.Quad3DLagrangeP1");
  m_supported_element_types.push_back("CF.Mesh.SF.Tetra3DLagrangeP1");
  m_supported_element_types.push_back("CF.Mesh.SF.Hexa3DLagrangeP1");

  m_elemtype_CGNS_to_CF[BAR_2  ] = "CF.Mesh.SF.Line";
  m_elemtype_CGNS_to_CF[TRI_3  ] = "CF.Mesh.SF.Triag";
  m_elemtype_CGNS_to_CF[QUAD_4 ] = "CF.Mesh.SF.Quad";
  m_elemtype_CGNS_to_CF[TETRA_4] = "CF.Mesh.SF.Tetra";
  m_elemtype_CGNS_to_CF[HEXA_8 ] = "CF.Mesh.SF.Hexa";

  m_elemtype_CF_to_CGNS["CF.Mesh.SF.Line1DLagrangeP1" ] = BAR_2;
  m_elemtype_CF_to_CGNS["CF.Mesh.SF.Line2DLagrangeP1" ] = BAR_2;
  m_elemtype_CF_to_CGNS["CF.Mesh.SF.Line3DLagrangeP1" ] = BAR_2;
  m_elemtype_CF_to_CGNS["CF.Mesh.SF.Triag2DLagrangeP1"] = TRI_3;
  m_elemtype_CF_to_CGNS["CF.Mesh.SF.Triag3DLagrangeP1"] = TRI_3;
  m_elemtype_CF_to_CGNS["CF.Mesh.SF.Quad2DLagrangeP1" ] = QUAD_4;
  m_elemtype_CF_to_CGNS["CF.Mesh.SF.Quad3DLagrangeP1" ] = QUAD_4;
  m_elemtype_CF_to_CGNS["CF.Mesh.SF.Tetra3DLagrangeP1"] = TETRA_4;
  m_elemtype_CF_to_CGNS["CF.Mesh.SF.Hexa3DLagrangeP1" ] = HEXA_8;
}

//////////////////////////////////////////////////////////////////////////////

} // CGNS
} // Mesh
} // CF
