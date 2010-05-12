#include <boost/foreach.hpp>

#include "Common/ObjectProvider.hpp"
#include "Mesh/P1/Quad2D.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace P1 {
  
////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Quad2D,
                         ElementType,
                         P1Lib >
aP1Quad2D_Provider ( "Mesh::P1::Quad2D" );


Quad2D::Quad2D() 
{
  m_shape=GeoShape::QUAD;
  m_shapeName=GeoShape::Convert::to_str(m_shape);
  m_order=1;
  m_nbNodes=4;
  m_nbFaces=4;
  m_nbEdges=0;
  
  // set size of connectivity order
  m_faces.resize(4);
  BOOST_FOREACH( std::vector<Uint>& face, m_faces )
    face.resize(2);
  // Note: edges must not set as they coincide with nodes

  m_faces[0][0]=0;    m_faces[0][1]=1;
  m_faces[1][0]=1;    m_faces[1][1]=2;
  m_faces[2][0]=2;    m_faces[2][1]=3;
  m_faces[3][0]=3;    m_faces[3][1]=0;
}

////////////////////////////////////////////////////////////////////////////////

} // P1
} // Mesh
} // CF
