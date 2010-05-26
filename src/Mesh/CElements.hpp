#ifndef CF_Mesh_CElements_hpp
#define CF_Mesh_CElements_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Mesh/MeshAPI.hpp"
#include "Mesh/ElementType.hpp"

namespace CF {
namespace Mesh {

  class GeoShape;

////////////////////////////////////////////////////////////////////////////////

/// CElements component class
/// This class stores information about elements 
/// to regions (CRegion)
/// @author Willem Deconinck, Tiago Quintino
class Mesh_API CElements : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CElements> Ptr;

public: // functions

  /// Contructor
  /// @param name of the component
  CElements ( const CName& name );

  /// Virtual destructor
  virtual ~CElements();

  /// Get the class name
  static std::string getClassName () { return "CElements"; }

  /// Configuration Options
  static void defineConfigOptions ( Common::OptionList& options ) {}

  // functions specific to the CElements component

  /// @return m_nameShape
  std::string getShapeName() const { return m_elementType->getShapeName(); }

  /// @return m_geoShape
  GeoShape::Type getShape() const  {  return m_elementType->getShape(); }

  /// @return number of faces
  Uint getNbFaces() const  {  return m_elementType->getNbFaces();  }

  /// @return number of edges
  Uint getNbEdges() const  {  return m_elementType->getNbEdges();  }

  /// @return m_nbNodes
  Uint getNbNodes() const  { return m_elementType->getNbNodes(); }

  /// @return m_order
  Uint getOrder() const { return m_elementType->getOrder(); }

  /// @return m_dimensionality
  Uint getDimensionality() const { return m_elementType->getDimensionality(); }

  /// @return faces connectivity
  /// faces[iFace][iNode]
  std::vector< std::vector< Uint > >& getFacesConnectivity() { return m_elementType->getFacesConnectivity(); }

  /// @return edges connectivity
  /// edges[iEdge][iNode]
  std::vector< std::vector< Uint > >& getEdgesConnectivity() { return m_elementType->getEdgesConnectivity(); }
    
  /// compute volume of the element given by its coordinates
  virtual Real computeVolume(const std::vector<CArray::Row>& coord) { return m_elementType->computeVolume(coord); } 

  /// set the element type
  void set_elementType(const std::string& etype_name);

  /// return the elementType
  boost::shared_ptr<ElementType>& get_elementType() { return m_elementType; }
  
private: // data
  
  boost::shared_ptr<ElementType> m_elementType;
  
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CElements_hpp
