#include "Mesh/Triag2D.hpp"
#include <boost/foreach.hpp>
#include "Math/RealMatrix.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  
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
  m_volume.resize(3);
  m_faces.resize(3);
  BOOST_FOREACH( std::vector<Uint>& face, m_faces )
    face.resize(2);
  // Note: edges must not set as they coincide with nodes
  
  m_volume[0]=0;
  m_volume[1]=1;
  m_volume[2]=2;
  
  m_faces[0][0]=0;    m_faces[0][1]=1;
  m_faces[1][0]=1;    m_faces[1][1]=2;
  m_faces[2][0]=2;    m_faces[2][1]=0;
}

////////////////////////////////////////////////////////////////////////////////

Real VolumeComputer<Triag2D>::computeVolume(const std::vector<RealVector*>& coord) 
{
  RealMatrix matrix(3,3);
  for (Uint i = 0; i < 3; ++i) {
    for (Uint j = 0; j < 3; ++j) {
      if (j > 0) {
        matrix(i,j) = (*coord[i])[j-1];
      }
      else {
        matrix(i,j) = 1.0;
      }
    }
  }
  
  const Real volume = 0.5*matrix.determ3();
  
  return volume;
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
