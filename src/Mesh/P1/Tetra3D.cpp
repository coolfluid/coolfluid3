#include <boost/foreach.hpp>

#include "Common/ObjectProvider.hpp"
#include "Mesh/P1/Tetra3D.hpp"
#include "Mesh/P1/Quad3D.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace P1 {
  
////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Tetra3D,
                         ElementType,
                         P1Lib >
aP1Tetra3D_Provider ( "P1-"+Tetra3D::getClassName() );

////////////////////////////////////////////////////////////////////////////////

Tetra3D::Tetra3D()
{
  m_shape=GeoShape::TETRA;
  m_shapeName=GeoShape::Convert::to_str(m_shape);
  m_order=1;
  m_nbNodes=4;
  m_dimension=3;
  m_dimensionality=3;
  m_nbEdges=6;
  
  // TODO:set faces
}

////////////////////////////////////////////////////////////////////////////////

} // P1
} // Mesh
} // CF
