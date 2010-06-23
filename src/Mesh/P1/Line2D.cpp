#include <boost/foreach.hpp>

#include "Common/ObjectProvider.hpp"
#include "Mesh/P1/Line2D.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace P1 {
  
////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Line2D,
                         ElementType,
                         P1Lib >
aP1Line2D_Provider ( "P1-"+Line2D::getClassName() );


Line2D::Line2D()
{
  m_shape=GeoShape::LINE;
  m_shapeName=GeoShape::Convert::to_str(m_shape);
  m_order=1;
  m_nbNodes=2;
  m_dimension=2;
  m_dimensionality=1;
  m_nbEdges=0;
  

}

//////////////////////////////////////////////////////////////////////

const Line2D& Line2D::instance() {
  static Line2D obj;
  return obj;
}
  
////////////////////////////////////////////////////////////////////////////////

} // P1
} // Mesh
} // CF
