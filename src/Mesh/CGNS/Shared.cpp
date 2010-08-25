#include "Mesh/CGNS/Shared.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace CGNS {
  
//////////////////////////////////////////////////////////////////////////////

Shared::Shared()
{
  m_supported_element_types.reserve(9);
  m_supported_element_types.push_back("Line1DLagrangeP1");
  m_supported_element_types.push_back("Line2DLagrangeP1");
  m_supported_element_types.push_back("Line3DLagrangeP1");
  m_supported_element_types.push_back("Triag2DLagrangeP1");
  m_supported_element_types.push_back("Triag3DLagrangeP1");
  m_supported_element_types.push_back("Quad2DLagrangeP1");
  m_supported_element_types.push_back("Quad3DLagrangeP1");
  m_supported_element_types.push_back("Tetra3DLagrangeP1");
  m_supported_element_types.push_back("Hexa3DLagrangeP1");

  m_elemtype_CGNS_to_CF[BAR_2  ] = "Line";
  m_elemtype_CGNS_to_CF[TRI_3  ] = "Triag";
  m_elemtype_CGNS_to_CF[QUAD_4 ] = "Quad";
  m_elemtype_CGNS_to_CF[TETRA_4] = "Tetra";
  m_elemtype_CGNS_to_CF[HEXA_8 ] = "Hexa";

  m_elemtype_CF_to_CGNS["Line1DLagrangeP1" ] = BAR_2;
  m_elemtype_CF_to_CGNS["Line2DLagrangeP1" ] = BAR_2;
  m_elemtype_CF_to_CGNS["Line3DLagrangeP1" ] = BAR_2;
  m_elemtype_CF_to_CGNS["Triag2DLagrangeP1"] = TRI_3;
  m_elemtype_CF_to_CGNS["Triag3DLagrangeP1"] = TRI_3;
  m_elemtype_CF_to_CGNS["Quad2DLagrangeP1" ] = QUAD_4;
  m_elemtype_CF_to_CGNS["Quad3DLagrangeP1" ] = QUAD_4;
  m_elemtype_CF_to_CGNS["Tetra3DLagrangeP1"] = TETRA_4;
  m_elemtype_CF_to_CGNS["Hexa3DLagrangeP1" ] = HEXA_8;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

} // CGNS
} // Mesh
} // CF
