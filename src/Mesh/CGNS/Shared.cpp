#include "Mesh/CGNS/Shared.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace CGNS {
  
//////////////////////////////////////////////////////////////////////////////

Shared::Shared()
{
  m_supported_element_types.reserve(4);
  m_supported_element_types.push_back("P1-Line1D");
  m_supported_element_types.push_back("P1-Line2D");
  m_supported_element_types.push_back("P1-Line3D");
  m_supported_element_types.push_back("P1-Triag2D");
  m_supported_element_types.push_back("P1-Triag3D");
  m_supported_element_types.push_back("P1-Quad2D");
  m_supported_element_types.push_back("P1-Quad3D");
//  m_supported_element_types.push_back("P1-Tetra3D");
  m_supported_element_types.push_back("P1-Hexa3D");

  m_elemtype_CGNS_to_CF[TRI_3  ] = "P1-Triag";
  m_elemtype_CGNS_to_CF[QUAD_4 ] = "P1-Quad";
  m_elemtype_CGNS_to_CF[TETRA_4] = "P1-Tetra";
  m_elemtype_CGNS_to_CF[HEXA_8 ] = "P1-Hexa";

  m_elemtype_CF_to_CGNS["P1-Triag2D"] = TRI_3;
  m_elemtype_CF_to_CGNS["P1-Triag3D"] = TRI_3;
  m_elemtype_CF_to_CGNS["P1-Quad2D" ] = QUAD_4;
  m_elemtype_CF_to_CGNS["P1-Quad3D" ] = QUAD_4;
  m_elemtype_CF_to_CGNS["P1-Tetra3D"] = TETRA_4;
  m_elemtype_CF_to_CGNS["P1-Hexa3D" ] = HEXA_8;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

} // CGNS
} // Mesh
} // CF
