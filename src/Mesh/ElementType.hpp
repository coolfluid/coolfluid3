#ifndef CF_Mesh_ElementType_hh
#define CF_Mesh_ElementType_hh

////////////////////////////////////////////////////////////////////////////////

#include "Common/StringOps.hpp"
#include "Mesh/GeoShape.hpp"
#include "Common/BasicExceptions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

  using namespace Common;

////////////////////////////////////////////////////////////////////////////////

/// This class represents the the data related to an ElementType
/// @author Tiago Quintino, Willem Deconinck
class Mesh_API ElementType {

public: // functions

  /// Default constructor without arguments
  ElementType();

  /// Default destructor
  ~ElementType();

public: // accessors

  /// @return m_nameShape
  std::string getShape() const { return m_nameShape; }

  /// @return m_geoShape
  GeoShape::Type getGeoShape() const  {  return m_geoShape; }

  /// @return m_nbElems
  Uint getNbElems() const  {  return m_nbElems; }

  /// @return m_startIdx
  Uint getStartIdx() const { return m_startIdx; }

  /// @return m_endIdx
  Uint getEndIdx() const  { return m_startIdx + m_nbElems; }

  /// @return number of faces
  Uint getNbFaces() const  {  return m_nbfaces;  }

  /// @return m_nbNodes
  Uint getNbNodes() const  { return m_nbNodes; }

  /// @return m_nbStates
  Uint getNbStates() const {  return m_nbStates; }

  /// @return m_geoOrder
  Uint getGeoOrder() const { return m_geoOrder; }

  /// @return m_solOrder
  Uint getSolOrder() const  { return m_solOrder; }

public: // mutators

  /// Sets m_nameShape
  void setShape(const std::string& nameShape) { m_nameShape = nameShape; }

  /// Sets m_geoShape
  void setGeoShape(const GeoShape::Type& geoShape)
  {
    m_geoShape = geoShape;
    setNbFaces(m_geoShape);
    setShape(GeoShape::Convert::to_str(m_geoShape));
  }
  
  /// Sets the nb faces
  void setNbFaces(const GeoShape::Type& geoShape)
  {
    switch(geoShape) {
      case GeoShape::LINE:
        m_nbfaces = 2; break;
      case GeoShape::TRIAG:
        m_nbfaces = 3; break;
      case GeoShape::QUAD:
        m_nbfaces = 4; break;
      case GeoShape::TETRA:
        m_nbfaces = 4; break;
      case GeoShape::PYRAM:
        m_nbfaces = 5; break;
      case GeoShape::PRISM:
        m_nbfaces = 5; break;
      case GeoShape::HEXA:
        m_nbfaces = 6; break;
      default:
        throw BadValue(FromHere(),"Shape not defined: " + Common::StringOps::to_str(geoShape));
    }
  }

  /// Sets m_nbElems
  void setNbElems(const Uint nbElems) {  m_nbElems = nbElems; }

  /// Sets m_startIdx
  void setStartIdx(const Uint startIdx) { m_startIdx = startIdx; }

  /// Sets m_nbNodes
  void setNbNodes(const Uint nbNodes) { m_nbNodes = nbNodes; }

  /// Sets m_nbStates
  void setNbStates(const Uint nbStates) { m_nbStates = nbStates; }

  /// Sets m_geoOrder
  void setGeoOrder(const Uint geoOrder) { m_geoOrder = geoOrder; }

  /// Sets m_solOrder
  void setSolOrder(const Uint solOrder) { m_solOrder = solOrder; }

private: // data

  /// the string identifying the shape of this type
  std::string   m_nameShape;
  /// the GeoShape::Type corresponding to the shape
  GeoShape::Type m_geoShape;
  /// the number of elements with this type int the Mesh
  Uint  m_nbElems;
  /// the start index on the ordered vector of elements for this type
  Uint m_startIdx;
  /// the number of nodes in this element type
  Uint m_nbNodes;
  /// the number of states in this element type
  Uint m_nbStates;
  /// the  geometric order of this element
  Uint m_geoOrder;
  /// the  solution order of this element
  Uint m_solOrder;
  /// number of faces
  Uint m_nbfaces;

}; // end of class ElementType

////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_ElementType_hh
