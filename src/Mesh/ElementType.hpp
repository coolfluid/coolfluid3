#ifndef CF_Mesh_ElementType_hpp
#define CF_Mesh_ElementType_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/range.hpp>

#include "Common/String/Conversion.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/ConcreteProvider.hpp"

#include "Math/RealVector.hpp"

#include "Mesh/GeoShape.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

  using namespace Common;

////////////////////////////////////////////////////////////////////////////////

template <typename T> class VolumeComputer;

////////////////////////////////////////////////////////////////////////////////

/// This class represents the the data related to an ElementType
/// @author Tiago Quintino, Willem Deconinck
class Mesh_API ElementType : public boost::noncopyable {

public: // functions

  typedef Common::ConcreteProvider < ElementType > PROVIDER;
  
  /// Type used to pass node coordinates of an element
  typedef std::vector<RealVector> NodesT;

  /// Stores connectivity information about the faces that form the cell boundary
  struct FaceConnectivity
  {
    /// Storage for index arrays
    typedef std::vector<Uint> IndicesT;
    
    /// Range of const indices
    typedef boost::iterator_range<IndicesT::const_iterator> RangeT;
    
    /// Index of the first node of each face, relative to the numbering of the parent cell
    IndicesT face_first_nodes;
    
    /// Number of nodes for each face
    IndicesT face_node_counts;
    
    /// Node indices for each face
    IndicesT face_nodes;
    
    /// Iterator range over the nodes of the given face
    RangeT face_node_range(const Uint face) const
    {
      if(face_first_nodes.empty())
        return boost::make_iterator_range(face_nodes.begin(), face_nodes.end());
      IndicesT::const_iterator begin = face_nodes.begin() + face_first_nodes[face];
      return boost::make_iterator_range(begin, begin + face_node_counts[face]);
    }
  };
  
  /// Default constructor without arguments
  ElementType();

  /// Default destructor
  virtual ~ElementType();

  static std::string type_name() { return "ElementType"; }
  
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
  virtual Real computeVolume(const NodesT& coord) const = 0;
  
  /// Return the face connectivity information
  virtual const FaceConnectivity& face_connectivity() const = 0;
  
  /// Return the face type for the given face
  virtual const ElementType& face_type(const Uint face) const = 0;
    
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

}; // end of class ElementType
  

////////////////////////////////////////////////////////////////////////////////

template <typename ETYPE>
struct IsElementType
{
  /// Return true if etype is of concrete type ETYPE
  bool operator()(const ElementType& etype)
  {
    return
        ( etype.shape()          == ETYPE::shape         &&
          etype.order()          == ETYPE::order         &&
          etype.dimension()      == ETYPE::dimension     &&
          etype.dimensionality() == ETYPE::dimensionality );
  }
};

////////////////////////////////////////////////////////////////////////////////

/// Evaluate a shape function
template<typename ShapeFunctionT, typename NodeValuesT, typename ResultT>
void eval(const RealVector& mapped_coord, const NodeValuesT& values, ResultT& result)
{
  RealVector sf(ShapeFunctionT::nb_nodes);
  ShapeFunctionT::shape_function(mapped_coord, sf);
  
  // Make sure result is zero
  result -= result;
  
  // Add node contributions
  for(Uint i = 0; i != ShapeFunctionT::nb_nodes; ++i)
    result += (values[i] * sf[i]);
}

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_ElementType_hpp
