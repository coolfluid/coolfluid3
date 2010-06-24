#ifndef CF_Mesh_ElementTypeBase_hpp
#define CF_Mesh_ElementTypeBase_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/StringOps.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/ConcreteProvider.hpp"

#include "Mesh/GeoShape.hpp"
#include "Mesh/CArray.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

  using namespace Common;

////////////////////////////////////////////////////////////////////////////////

template <typename T> class VolumeComputer;

////////////////////////////////////////////////////////////////////////////////

/// This class represents the the data related to an ElementTypeBase
/// @author Tiago Quintino, Willem Deconinck
class Mesh_API ElementTypeBase {

public: // functions

  typedef Common::ConcreteProvider < ElementTypeBase > PROVIDER;

  /// Default constructor without arguments
  ElementTypeBase();

  /// Default destructor
  ~ElementTypeBase();

  static std::string getClassName() { return "ElementTypeBase"; }
  
  /// @return m_nameShape
  std::string shape_name() const { return EnumT<GeoShape>::to_str( m_shape ); }

  /// @return m_geoShape
  GeoShape::Type shape() const  {  return m_shape; }

  /// @return number of faces
  Uint nb_faces() const  {  return m_nb_faces;  }

  /// @return number of edges
  Uint nb_edges() const  {  return m_nb_edges;  }

  /// @return m_nbNodes
  Uint nb_nodes() const  { return m_nb_nodes; }

  /// @return m_order
  Uint order() const { return m_order; }

  /// @return m_dimensionality
  Uint dimensionality() const { return m_dimensionality; }

  /// @return m_dimension
  Uint dimension() const { return m_dimension; }

  /// get a string with the element type name
  virtual std::string getElementTypeName() const = 0;

  /// compute volume given coordinates
  virtual Real computeVolume(const std::vector<CArray::Row>& coord) const = 0;
  
protected: // data

  /// the GeoShape::Type corresponding to the shape
  GeoShape::Type m_shape;
  /// the number of nodes in this element type
  Uint m_nb_nodes;
  /// the  geometric order of this element
  Uint m_order;
  /// the dimension of the coordinates of this elements
  Uint m_dimension;
  /// the dimensionality of the element
  Uint m_dimensionality;
  /// number of edges
  Uint m_nb_edges;
  /// number of edges
  Uint m_nb_faces;

}; // end of class ElementTypeBase
  

////////////////////////////////////////////////////////////////////////////////

template <typename ETYPE>
struct IsElemType
{
  bool operator()(const ElementTypeBase& etype)
  {
    return
        ( etype.shape()          == ETYPE::shape()         &&
          etype.order()          == ETYPE::order()         &&
          etype.dimension()      == ETYPE::dimension()     &&
          etype.dimensionality() == ETYPE::dimensionality() );
  }
};

////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_ElementTypeBase_hpp
