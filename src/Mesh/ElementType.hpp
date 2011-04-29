// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_ElementType_hpp
#define CF_Mesh_ElementType_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/range.hpp>

#include "Common/StringConversion.hpp"
#include "Common/Component.hpp"

#include "Math/MatrixTypes.hpp"

#include "Mesh/ElementData.hpp"
#include "Mesh/GeoShape.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

template <typename T> class VolumeComputer;

////////////////////////////////////////////////////////////////////////////////

/// This class represents the the data related to an ElementType
/// @author Tiago Quintino
/// @author Willem Deconinck
class Mesh_API ElementType : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr< ElementType > Ptr;
  typedef boost::shared_ptr< ElementType const> ConstPtr;

public: // functions

  /// Type used to pass node coordinates of an element.
  /// Each row of the matrix represents the coordinates of a node
  typedef RealMatrix NodesT;

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
  ElementType( const std::string& name = type_name() );

  /// Default destructor
  virtual ~ElementType();

  static std::string type_name() { return "ElementType"; }
  
  /// @return m_nameShape
  std::string shape_name() const { return Mesh::GeoShape::Convert::instance().to_str( m_shape ); }

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

  virtual std::string builder_name() const = 0;

  /// compute volume given coordinates
  /// @note Only in elements of dimensionality == dimension will
  /// the volume be different from zero
  virtual Real compute_volume(const NodesT& coord) const = 0;

  /// compute area using given coordinates.
  /// @note Only in elements of dimensionality == dimensionality-1
  /// will the area be different from zero
  virtual Real compute_area(const NodesT& coord) const = 0;

  /// compute the normal to the element. The direction will be taken according
  /// to the order of the coordinates
  virtual void compute_normal(const NodesT& coord, RealVector& normal) const;
  
  /// compute centroid of element given coordinates
  virtual void compute_centroid(const NodesT& coord , RealVector& centroid) const;
  
  
  /// Return the face connectivity information
  virtual const FaceConnectivity& face_connectivity() const = 0;
  
  /// Return the face type for the given face
  virtual const ElementType& face_type(const Uint face) const = 0;

  /// @return if the coordinate is in the element with given nodes
  /// @param [in] coord  the coordinates that will be checked
  /// @param [in] nodes  the nodes of the element
  virtual bool is_coord_in_element(const RealVector& coord, const NodesT& nodes) const = 0;

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

}; // ElementType

////////////////////////////////////////////////////////////////////////////////

template <typename ETYPE>
struct IsElementType
{
  /// Return true if etype is of concrete type ETYPE
  bool operator()(const ElementType& etype)
  {
    return
//        ( etype.shape()          == ETYPE::shape         &&
//          etype.order()          == ETYPE::order         &&
//          etype.nb_nodes()       == ETYPE::nb_nodes      &&
//          etype.dimension()      == ETYPE::dimension     &&
//          etype.dimensionality() == ETYPE::dimensionality )
//        &&
        ( etype.derived_type_name() == ETYPE::type_name() );
  }
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_ElementType_hpp
