// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SF_Hexa_hpp
#define CF_Mesh_SF_Hexa_hpp

#include "Mesh/ElementType.hpp"

#include "Mesh/SF/LibSF.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in hexahedral element.
/// @author Willem Deconinck
/// @author Tiago Quintino
/// @author Bart Janssens
template <typename SF>
struct Hexa : public Mesh::ElementType {
public:

  typedef boost::shared_ptr< Hexa > Ptr;
  typedef boost::shared_ptr< Hexa const> ConstPtr;

  Hexa(const std::string& name = type_name()) : Mesh::ElementType(name)
  {
    m_shape = shape;
    m_dimension = dimension;
    m_nb_faces = nb_faces;
    m_nb_edges = nb_edges;
    m_nb_nodes = nb_nodes;
    m_dimensionality = dimensionality;
    m_order = order;
    BOOST_STATIC_ASSERT(SF::shape == shape);
  }

  static const Uint dimension = DIM_3D;
  static const Uint dimensionality = SF::dimensionality;

  /// @return m_geoShape
  static const GeoShape::Type shape = GeoShape::Hexa;

  /// @return number of faces
  static const Uint nb_faces = 6;

  /// @return number of edges
  static const Uint nb_edges = 12;

  /// Number of nodes
  static const Uint nb_nodes = SF::nb_nodes;

  /// Order of the shape function
  static const Uint order = SF::order;

  /// Types for the matrices used
  typedef Eigen::Matrix<Real, dimension, 1>              CoordsT;
  typedef Eigen::Matrix<Real, dimensionality, 1>         MappedCoordsT;
  typedef Eigen::Matrix<Real, nb_nodes, dimension>       NodeMatrixT;
  typedef Eigen::Matrix<Real, 1, nb_nodes>               ShapeFunctionsT;
  typedef Eigen::Matrix<Real, dimensionality, nb_nodes>  MappedGradientT;
  typedef Eigen::Matrix<Real, dimensionality, dimension> JacobianT;

  // Delegation of shape_function and shape_function_gradient to template parameter

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
};

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF

#endif // CF_Mesh_SF_Hexa_hpp
