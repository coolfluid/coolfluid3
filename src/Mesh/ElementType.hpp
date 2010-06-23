#ifndef CF_Mesh_ElementType_hpp
#define CF_Mesh_ElementType_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/StringOps.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/ConcreteProvider.hpp"

#include "Math/RealVector.hpp"

#include "Mesh/GeoShape.hpp"
#include "Mesh/CArray.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

  using namespace Common;

////////////////////////////////////////////////////////////////////////////////

template <typename T> class VolumeComputer;

////////////////////////////////////////////////////////////////////////////////

/// This class represents the the data related to an ElementType
/// @author Tiago Quintino, Willem Deconinck
class Mesh_API ElementType {

public: // functions

  typedef Common::ConcreteProvider < ElementType > PROVIDER;

  /// Default constructor without arguments
  ElementType();

  /// Default destructor
  ~ElementType();

  static std::string getClassName() { return "ElementType"; }

  virtual std::string getElementTypeName() = 0;

public: // accessors

  typedef std::pair<const ElementType*, const Uint*> FacePair;
  
  struct Face
  {
    Face(const boost::shared_ptr<ElementType>& t, const std::vector<Uint>& n)
      : faceType(t), nodes(n) {}
    boost::shared_ptr<ElementType> faceType;
    std::vector<Uint> nodes;
  };
  
  struct FaceStruct
  {
    FaceStruct(const ElementType* t, const Uint* n)
    : type(t), nodes(n) {}
    const ElementType* type;
    const Uint* nodes;
  };
  
  

  /// @return m_nameShape
  std::string getShapeName() const { return m_shapeName; }

  /// @return m_geoShape
  GeoShape::Type getShape() const  {  return m_shape; }

  /// @return number of faces
  Uint getNbFaces() const  {  return m_faces.size();  }

  /// @return number of edges
  Uint getNbEdges() const  {  return m_nbEdges;  }

  /// @return m_nbNodes
  Uint getNbNodes() const  { return m_nbNodes; }

  /// @return m_order
  Uint getOrder() const { return m_order; }

  /// @return m_dimensionality
  Uint getDimensionality() const { return m_dimensionality; }

  /// @return m_dimension
  Uint getDimension() const { return m_dimension; }

  /// @return faces connectivity
  /// faces[iFace][iNode]
  const std::vector<Face>& getFaces() const { return m_faces; }
  
  /// compute volume given coordinates
  virtual Real computeVolume(const std::vector<CArray::Row>& coord) const =0; 

  /////////////////////////////////////////////
  // static functions
  
  /// @return m_nameShape
  static std::string static_get_shapeName();
  
  /// @return m_geoShape
  static GeoShape::Type static_get_shape();
  
  /// @return number of faces
  static Uint static_get_nbFaces();
  
  /// @return number of edges
  static Uint static_get_nbEdges();
  
  /// @return m_nbNodes
  static Uint static_get_nbNodes();

  /// @return m_order
  static Uint static_get_Order();
  
  /// @return m_dimensionality
  static Uint static_get_dimensionality();
  
  /// @return m_dimension
  static Uint static_get_dimension();
  
  /// @return faces connectivity
  /// faces[iFace][iNode]
  static const std::vector<Face>& static_get_faces();
  
  
protected: // data

  /// the string identifying the shape of this type
  std::string   m_shapeName;
  /// the GeoShape::Type corresponding to the shape
  GeoShape::Type m_shape;
  /// the number of nodes in this element type
  Uint m_nbNodes;
  /// the  geometric order of this element
  Uint m_order;
  /// the dimension of the coordinates of this elements
  Uint m_dimension;
  /// the dimensionality of the element
  Uint m_dimensionality;
  /// number of edges
  Uint m_nbEdges;

  /// vector of faces
  std::vector<Face> m_faces;

}; // end of class ElementType
  

////////////////////////////////////////////////////////////////////////////////

template <typename ETYPE>
struct IsElemType
{
  bool operator()(const ElementType& etype)
  {
    ETYPE thistype;
    return
        ( etype.getShape()          == thistype.getShape()         &&
          etype.getOrder()          == thistype.getOrder()         &&
          etype.getDimension()      == thistype.getDimension()     &&
          etype.getDimensionality() == thistype.getDimensionality() );
  }
};

////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_ElementType_hpp
