#include <boost/foreach.hpp>

#include "Common/ObjectProvider.hpp"
#include "Mesh/P1/Line1D.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace P1 {
  
////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Line1D,
                         ElementType,
                         P1Lib >
aP1Line1D_Provider ( Line1D::getFullName() );


Line1D::Line1D()
{
  m_shape=GeoShape::LINE;
  m_shapeName=GeoShape::Convert::to_str(m_shape);
  m_order=1;
  m_nbNodes=2;
  m_dimensionality=1;
  m_nbFaces=2;
  m_nbEdges=0;
  
  // set size of connectivity order
  m_faces.resize(2);
  BOOST_FOREACH( std::vector<Uint>& face, m_faces )
    face.resize(2);
  // Note: edges must not set as they coincide with nodes

  
  m_faces[0][0]=0;
  m_faces[1][0]=1;
}

////////////////////////////////////////////////////////////////////////////////

} // P1
} // Mesh
} // CF
