#include <boost/foreach.hpp>

#include "Common/ObjectProvider.hpp"
#include "Mesh/P1/Tetra3D.hpp"
#include "Mesh/P1/Triag3D.hpp"

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
    
  // set faces
  m_faces.reserve(4);
  boost::shared_ptr<ElementType> triag(new Triag3D);
  std::vector<Uint> nodes(3);

  nodes[0]=0;   nodes[1]=2;   nodes[2]=1;    m_faces.push_back( Face(triag,nodes));
  nodes[0]=0;   nodes[1]=1;   nodes[2]=3;    m_faces.push_back( Face(triag,nodes));
  nodes[0]=1;   nodes[1]=2;   nodes[2]=3;    m_faces.push_back( Face(triag,nodes));
  nodes[0]=0;   nodes[1]=3;   nodes[2]=2;    m_faces.push_back( Face(triag,nodes));
}

////////////////////////////////////////////////////////////////////////////////

} // P1
} // Mesh
} // CF
