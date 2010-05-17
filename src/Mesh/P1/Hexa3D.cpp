#include <boost/foreach.hpp>

#include "Common/ObjectProvider.hpp"
#include "Mesh/P1/Hexa3D.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace P1 {
  
////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Hexa3D,
                         ElementType,
                         P1Lib >
aP1Hexa3D_Provider ( Hexa3D::getFullName() );

////////////////////////////////////////////////////////////////////////////////

Hexa3D::Hexa3D() 
{
  m_shape=GeoShape::HEXA;
  m_shapeName=GeoShape::Convert::to_str(m_shape);
  m_order=1;
  m_nbNodes=8;
  m_dimensionality=3;
  m_nbFaces=6;
  m_nbEdges=8;
  
  // set size of connectivity order
  m_faces.resize(m_nbFaces);
  BOOST_FOREACH( std::vector<Uint>& face, m_faces )
    face.resize(4);

//  m_faces[0][0]=0;    m_faces[0][1]=1;
//  m_faces[1][0]=1;    m_faces[1][1]=2;
//  m_faces[2][0]=2;    m_faces[2][1]=3;
//  m_faces[3][0]=3;    m_faces[3][1]=0;
}

////////////////////////////////////////////////////////////////////////////////

} // P1
} // Mesh
} // CF
