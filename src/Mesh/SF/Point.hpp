// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SF_Point_hpp
#define CF_Mesh_SF_Point_hpp

#include "Mesh/ElementType.hpp"

#include "Mesh/SF/LibSF.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P1 (Pointar)
/// Point element.
/// @author Willem Deconinck
/// @author Tiago Quintino
/// @author Bart Janssens
template <Uint DIM, typename SF>
class Point : public Mesh::ElementType {
public:

  typedef boost::shared_ptr< Point > Ptr;
  typedef boost::shared_ptr< Point const> ConstPtr;

  Point(const std::string& name = type_name()) : Mesh::ElementType(name)
  {
    m_shape = shape;
    m_dimension = DIM;
    m_nb_faces = nb_faces;
    m_nb_edges = nb_edges;
    m_nb_nodes = SF::nb_nodes;
    m_dimensionality = SF::dimensionality;
    m_order = SF::order;
    BOOST_STATIC_ASSERT(SF::shape == shape);
  }

  /// @return m_geoShape
  static const GeoShape::Type shape = GeoShape::POINT;

  /// @return number of faces
  static const Uint nb_faces = 0;

  /// @return number of edges
  static const Uint nb_edges = 0;

  /// Number of nodes
  static const Uint nb_nodes = SF::nb_nodes;

  /// Order of the shape function
  static const Uint order = SF::order;

  /// Types for the matrices used
  typedef Eigen::Matrix<Real, DIM, 1> CoordsT;
  typedef Eigen::Matrix<Real, SF::dimensionality, 1> MappedCoordsT;
  typedef Eigen::Matrix<Real, SF::nb_nodes, DIM> NodeMatrixT;
  typedef Eigen::Matrix<Real, 1, SF::nb_nodes> ShapeFunctionsT;
  typedef Eigen::Matrix<Real, SF::dimensionality, SF::nb_nodes> MappedGradientT;
  typedef Eigen::Matrix<Real, SF::dimensionality, DIM> JacobianT;

  // Delegation of shape_function_value and shape_function_gradient to template parameter

  /// Shape function reference
  virtual const ShapeFunction& shape_function() const
  {
    const static SF shape_function_obj;
    return shape_function_obj;
  }

  /// Compute the shape functions corresponding to the given
  /// mapped coordinates
  /// @param mappedCoord The mapped coordinates
  /// @param shapeFunc Vector storing the result
  static void shape_function_value(const MappedCoordsT& mappedCoord, ShapeFunctionsT& shapeFunc) { SF::compute_value(mappedCoord,shapeFunc); }

  /// Compute the gradient with respect to mapped coordinates, i.e. parial derivatives are in terms of the
  /// mapped coordinates. The result needs to be multiplied with the inverse jacobian to get the result in real
  /// coordinates.
  /// @param mappedCoord The mapped coordinates where the gradient should be calculated
  /// @param result Storage for the resulting gradient matrix
  static void shape_function_gradient(const MappedCoordsT& mappedCoord, MappedGradientT& result) { SF::compute_gradient(mappedCoord,result); }

  /// Compute the centroid
  /// @note for a point, the centroid is the point itself
  virtual void compute_centroid(const NodesT& coord, RealVector& centroid ) const { centroid = coord.row(0); }

  /// Compute the centroid
  /// @note for a point, the centroid is the point itself
  virtual bool is_coord_in_element(const RealVector& coord, const NodesT& nodes) const { return coord == nodes.row(0); }

};

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF

#endif /* CF_Mesh_SF_Point1DLagrangeP1 */
