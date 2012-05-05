// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_ElementType_hpp
#define cf3_mesh_ElementType_hpp

////////////////////////////////////////////////////////////////////////////////

//#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/range.hpp>

#include "math/MatrixTypes.hpp"

#include "mesh/Entities.hpp"
#include "mesh/ShapeFunction.hpp"
#include "mesh/ShapeFunctionT.hpp"
#include "mesh/GeoShape.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////

struct ElementTypeFaceConnectivity;

/// This class represents the the data related to an ElementType
/// @author Tiago Quintino
/// @author Willem Deconinck
class Mesh_API ElementType : public common::Component {

public: // typedefs




  typedef ElementTypeFaceConnectivity FaceConnectivity;
public: // functions

  /// @name Constructor / Destructor / Type name
  //  ------------------------------------------
  //@{

  /// Default constructor without arguments
  ElementType( const std::string& name ) : common::Component(name) {}

  /// Default destructor
  virtual ~ElementType() {}

  static std::string type_name() { return "ElementType"; }

  // @}

  /// @name Accessor functions
  //  ------------------------
  //@{

  /// @return shape as string
  std::string shape_name() const { return mesh::GeoShape::Convert::instance().to_str( m_shape ); }

  /// @return shape as enum
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

  /// @return the shape function defining this geometric element
  virtual const ShapeFunction& shape_function() const = 0;

  /// @return the face connectivity information
  virtual const FaceConnectivity& faces() const = 0;

  /// @return the face type for the given face
  virtual const ElementType& face_type(const Uint face) const = 0;

  //@}

  /// @name Computation functions
  //  ---------------------------
  //@{

  /// Compute Mapped Coordinates, a.k.a. local coordinates
  /// @param [in] coord     the coordinates to be mapped
  /// @param [in] nodes     coordinates of the element nodes (nb_nodes x dimension)
  /// @return mapped_coord
  virtual RealVector mapped_coordinate(const RealVector& coord, const RealMatrix& nodes) const = 0;

  /// Compute Mapped Coordinates, a.k.a. local coordinates
  /// @param [in]  coord         coordinates to be mapped
  /// @param [in]  nodes         coordinates of the element nodes (nb_nodes x dimension)
  /// @param [out] mapped_coord  result
  virtual void compute_mapped_coordinate(const RealVector& coord, const RealMatrix& nodes, RealVector& mapped_coord) const = 0;

  /// Compute the determinant of the jacobian dX/dKSI
  /// @param [in] mapped_coord  coordinates in mapped space (dimensionality x 1)
  /// @param [in] nodes         coordinates of the element nodes (nb_nodes x dimension)
  /// @return jacobian determinant
  virtual Real jacobian_determinant(const RealVector& mapped_coord, const RealMatrix& nodes) const = 0;

  /// Compute the jacobian of the transformation dX/dKSI
  /// @warning This convention is transposed of usual definition. Here used is:
  /// \f{eqnarray*}{
  ///   \frac{\partial x}{\partial \xi  } & \frac{\partial y}{\partial \xi  } & \frac{\partial z}{\partial \xi  } \\
  ///   \frac{\partial x}{\partial \eta } & \frac{\partial y}{\partial \eta } & \frac{\partial z}{\partial \eta } \\
  ///   \frac{\partial x}{\partial \zeta} & \frac{\partial y}{\partial \zeta} & \frac{\partial z}{\partial \zeta}
  /// \f}
  /// @param [in]  mapped_coord   coordinates in mapped space (dimensionality x 1)
  /// @param [in]  nodes          coordinates of the element nodes (nb_nodes x dimension)
  /// @param [out] jacobian       jacobian (size = dimensionality x dimension)
  virtual void compute_jacobian(const RealVector& mapped_coord, const RealMatrix& nodes, RealMatrix& jacobian) const = 0;

  /// Compute the jacobian of the transformation
  /// @param [in] mapped_coord  coordinates in mapped space (dimensionality x 1)
  /// @param [in] nodes         coordinates of the element nodes (nb_nodes x dimension)
  /// @return jacobian (size = dimensionality x dimension)
  virtual RealMatrix jacobian(const RealVector& mapped_coord, const RealMatrix& nodes) const = 0;

  /// Compute the adjoint of the jacobian of the transformation
  /// Useful for computation of inverse jacobian = 1/jacobian_determinant * jacobian_adjoint
  /// @param [in]  mapped_coord      coordinates in mapped space (dimensionality x 1)
  /// @param [in]  nodes             coordinates of the element nodes (nb_nodes x dimension)
  /// @param [out] jacobian adjoint  jacobianadjoint (size = dimensionality x dimension)
  virtual void compute_jacobian_adjoint(const RealVector& mapped_coord, const RealMatrix& nodes, RealMatrix& jacobian_adjoint) const = 0;

  /// compute volume given coordinates
  /// @param [in] nodes  coordinates of the element nodes (nb_nodes x dimension)
  /// @note Only in elements of (dimensionality == dimension) will
  /// the volume be different from zero
  virtual Real volume(const RealMatrix& nodes) const = 0;

  /// compute area using given coordinates.
  /// @param [in] nodes  coordinates of the element nodes (nb_nodes x dimension)
  /// @note Only in elements of dimensionality == dimensionality-1
  /// will the area be different from zero
  virtual Real area(const RealMatrix& nodes) const = 0;

  /// Compute the unit-normal to the face-element. The direction will be taken according
  /// to the order of the coordinates
  virtual void compute_normal(const RealMatrix& nodes, RealVector& normal) const = 0;

  /// compute centroid of element given coordinates
  /// @param [in]  nodes      coordinates of the element nodes (nb_nodes x dimension)
  /// @param [out] centroid   coordinates of the centroid
  virtual void compute_centroid(const RealMatrix& nodes , RealVector& centroid) const = 0;

  /// @return if the coordinate is in the element with given nodes
  /// @param [in] coord  the coordinates that will be checked
  /// @param [in] nodes  the nodes of the element
  virtual bool is_coord_in_element(const RealVector& coord, const RealMatrix& nodes) const = 0;

  /// Compute the jacobian of the plane or section of the element.
  /// The section is given by a mapped coordinate, and a direction perpendicular
  /// to the plane.
  /// Only elements with the dimension == dimensionality implement this function
  /// @param [in] mapped_coord coordinates in mapped space (dimensionality x 1)
  /// @param [in] nodes        coordinates of the element nodes (nb_nodes x dimension)
  /// @param [in] orientation  direction normal to the plane
  /// @return vector in mapped space scaled with the jacobian of the
  ///         section (not the volume).
  virtual RealVector plane_jacobian_normal(const RealVector& mapped_coord,
                                           const RealMatrix& nodes,
                                           const CoordRef orientation) const = 0;

  /// Compute the jacobian of the plane or section of the element.
  /// The section is given by a mapped coordinate, and a direction perpendicular
  /// to the plane.
  /// Only elements with the dimension == dimensionality implement this function
  /// @param [in]  mapped_coord  coordinates in mapped space (dimensionality x 1)
  /// @param [in]  nodes         coordinates of the element nodes (nb_nodes x dimension)
  /// @param [in]  orientation   direction normal to the plane
  /// @param [out] result        vector in mapped space scaled with the jacobian of the
  ///                           section (not the volume).
  virtual void compute_plane_jacobian_normal(const RealVector& mapped_coord,
                                             const RealMatrix& nodes,
                                             const CoordRef orientation,
                                             RealVector& result) const = 0;

  //@}

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

/// Stores connectivity information about the faces that form the cell boundary
struct ElementTypeFaceConnectivity
{
  /// Range of const indices
  typedef boost::iterator_range<std::vector<Uint>::const_iterator> RangeT;

  /// Index of the first node of each face, relative to the numbering of the parent cell
  std::vector<Uint> displs;

  /// Number of nodes for each face
  std::vector<Uint> stride;

  /// Node indices for each face
  std::vector<Uint> nodes;

  /// Iterator range over the nodes of the given face
  RangeT nodes_range(const Uint face) const
  {
    if(displs.empty())
      return boost::make_iterator_range(nodes.begin(), nodes.end());
    std::vector<Uint>::const_iterator begin = nodes.begin() + displs[face];
    return boost::make_iterator_range(begin, begin + stride[face]);
  }
};

////////////////////////////////////////////////////////////////////////////////

template <typename ETYPE>
struct IsElementType
{
  /// Return true if etype is of concrete type ETYPE
  bool operator()(const ElementType& etype)
  {
    return ETYPE::dimension == etype.dimension() && is_not_null(dynamic_cast<const ShapeFunctionT<typename ETYPE::SF>*>(&etype.shape_function()));
  }

  bool operator()(const Handle< Entities >& component)
  {
    return operator()(component->element_type());
  }

  bool operator()(const Entities& component)
  {
    return operator()(component.element_type());
  }
};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_ElementType_hpp
