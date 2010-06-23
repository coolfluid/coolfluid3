#ifndef CF_Mesh_P1_Triag2D_hpp
#define CF_Mesh_P1_Triag2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/assign/list_of.hpp>

#include "Math/RealMatrix.hpp"

#include "Mesh/ElementType.hpp"

#include "Mesh/P1/P1API.hpp"
#include "Mesh/P1/Line2D.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace P1 {
    class Triag2D;
}

////////////////////////////////////////////////////////////////////////////////
  
template<>
class Mesh_API VolumeComputer<P1::Triag2D> 
{
public:
  static Real computeVolume(const std::vector<CArray::Row>& coord) ; 
};

//////////////////////////////////////////////////////////////////////////////

namespace P1 {
  
/// This class defines a 2D Triangle mesh element
/// @author Willem Deconinck
class P1_API Triag2D : public ElementType
{
public:
  
  typedef VolumeComputer<Triag2D> VolumeComputerType;
  
  /// constructor
  Triag2D();
  
  /// Gets the Class name
  static std::string getClassName() { return "Triag2D"; }

  /// Get the full name defining this element type uniquely
  virtual std::string getElementTypeName() { return "P1-Triag2D"; }
  
  Real computeVolume(const std::vector<CArray::Row>& coord) const 
  { 
    return VolumeComputerType::computeVolume(coord); 
  }

public:
    
  /// @return m_geoShape
  static const GeoShape::Type shape;
  
  /// @return m_nameShape
  static const std::string shapeName;

  /// @return number of faces
  static const Uint nbFaces;
  
  /// @return number of edges
  static const Uint nbEdges;
  
  /// @return m_nbNodes
  static const Uint nbNodes;
  
  /// @return m_order
  static const Uint order;
  
  /// @return m_dimensionality
  static const Uint dimensionality;
  
  /// @return m_dimension
  static const Uint dimension;
  
  /// @return faces connectivity
  /// faces[iFace][iNode]
  
  static const Uint face1_nodes[];
  static const Uint face2_nodes[];
  static const Uint face3_nodes[];
  static const std::vector<FaceStruct> faces;
  
private: // dummy static data
  
  static const Line2D* line;
  static const FaceStruct dummy_faces[];
}; // end Triag2D
  
} // namespace P1

////////////////////////////////////////////////////////////////////////////////

Real VolumeComputer<P1::Triag2D>::computeVolume(const std::vector<CArray::Row>& coord)
{
  return 0.5*((coord[1][XX]-coord[0][XX])*(coord[2][YY]-coord[0][YY])
                -(coord[2][XX]-coord[0][XX])*(coord[1][YY]-coord[0][YY]));
}
  
////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_P1_Triag2D_hpp
