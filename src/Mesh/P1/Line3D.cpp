#include <boost/foreach.hpp>

#include "Common/ObjectProvider.hpp"
#include "Mesh/P1/Line3D.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace P1 {
  
////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Line3D,
                         ElementType,
                         P1Lib >
aP1Line3D_Provider ( "P1-"+Line3D::getClassName() );


Line3D::Line3D()
{
  m_shape=GeoShape::LINE;
  m_shapeName=GeoShape::Convert::to_str(m_shape);
  m_order=1;
  m_nbNodes=2;
  m_dimension=3;
  m_dimensionality=1;
  m_nbEdges=0;

}

////////////////////////////////////////////////////////////////////////////////

} // P1
} // Mesh
} // CF
