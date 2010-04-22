#include "Mesh/P1/Triag2D.hpp"
#include <boost/foreach.hpp>

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace P1 {
  
////////////////////////////////////////////////////////////////////////////////

Triag2D::Triag2D() 
{
  m_shape=GeoShape::TRIAG;
  m_shapeName=GeoShape::Convert::to_str(m_shape);
  m_order=1;
  m_nbNodes=3;
  m_nbFaces=3;
  m_nbEdges=0;
  
  // set size of connectivity order
  m_faces.resize(3);
  BOOST_FOREACH( std::vector<Uint>& face, m_faces )
    face.resize(2);
  // Note: edges must not set as they coincide with nodes

  
  m_faces[0][0]=0;    m_faces[0][1]=1;
  m_faces[1][0]=1;    m_faces[1][1]=2;
  m_faces[2][0]=2;    m_faces[2][1]=0;
}

////////////////////////////////////////////////////////////////////////////////

} // P1
} // Mesh
} // CF
