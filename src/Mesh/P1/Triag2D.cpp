#include <boost/foreach.hpp>

#include "Common/ObjectProvider.hpp"
#include "Mesh/P1/Triag2D.hpp"
#include "Mesh/P1/Line2D.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace P1 {
  
////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Triag2D,
                         ElementType,
                         P1Lib >
aP1Triag2D_Provider ( "P1-"+Triag2D::getClassName() );


Triag2D::Triag2D() 
{
  m_shape=shape;
  m_shapeName=shapeName;
  m_order=order;
  m_nbNodes=nbNodes;
  m_dimension=dimension;
  m_dimensionality=dimensionality;
  m_nbEdges=nbEdges;
  
  // set faces
  m_faces.reserve(3);
  boost::shared_ptr<ElementType> line(new Line2D);
  std::vector<Uint> nodes(2);

  nodes[0]=0;   nodes[1]=1;   m_faces.push_back( Face(line,nodes));
  nodes[0]=1;   nodes[1]=2;   m_faces.push_back( Face(line,nodes));
  nodes[0]=2;   nodes[1]=0;   m_faces.push_back( Face(line,nodes));
}
  
//////////////////////////////////////////////////////////////////////
  
const std::string                     Triag2D::shapeName = "Triag";

const GeoShape::Type Triag2D::shape = GeoShape::TRIAG;
    
const Uint Triag2D::nbFaces = 3;
  
const Uint Triag2D::nbEdges = 3;
  
const Uint Triag2D::nbNodes = 3;
  
const Uint Triag2D::order = 1;
  
const Uint Triag2D::dimensionality = 2;
  
const Uint Triag2D::dimension = 2; 
  
const Uint                            Triag2D::face1_nodes[dimension] = { 0, 1};
const Uint                            Triag2D::face2_nodes[dimension] = { 1, 2};
const Uint                            Triag2D::face3_nodes[dimension] = { 2, 0};

const Line2D*                         Triag2D::line = new Line2D();

const Triag2D::FaceStruct             Triag2D::dummy_faces[nbFaces] = { FaceStruct(line,face1_nodes) , 
                                                                        FaceStruct(line,face2_nodes) , 
                                                                        FaceStruct(line,face3_nodes) };
const std::vector<ElementType::FaceStruct>  Triag2D::faces = std::vector<FaceStruct>(dummy_faces,dummy_faces+nbFaces);
  
  
} // P1
} // Mesh
} // CF
