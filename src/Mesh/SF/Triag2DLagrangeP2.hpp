// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SF_Triag2DLagrangeP2_hpp
#define CF_Mesh_SF_Triag2DLagrangeP2_hpp

#include "Math/MatrixTypes.hpp"
#include "Math/MathConsts.hpp"

#include "Mesh/Triag2D.hpp"

#include "Mesh/SF/LibSF.hpp"
#include "Mesh/SF/SFTriagLagrangeP2.hpp"

namespace CF {
namespace Mesh {
namespace SF {

/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P1 (linear)
/// triangular element.
/// @author Andrea Lani
/// @author Geoffrey Deliege
/// @author Tiago Quintino
/// @author Bart Janssens
struct MESH_SF_API Triag2DLagrangeP2  : public Triag2D
{
  Triag2DLagrangeP2(const std::string& name = type_name());

  static std::string type_name() { return "Triag2DLagrangeP2"; }

  virtual std::string builder_name() const { return LibSF::library_namespace()+"."+type_name(); }

  /// typedef for the supporting geometry
  typedef Triag2D Support;

  /// Number of nodes
  static const Uint nb_nodes = 6;

  /// Order of the shape function
  static const Uint order = 2;
  
  /// Types for the matrices used
  typedef Eigen::Matrix<Real, dimension, 1> CoordsT;
  typedef Eigen::Matrix<Real, dimensionality, 1> MappedCoordsT;
  typedef Eigen::Matrix<Real, nb_nodes, dimension> NodeMatrixT;
  typedef Eigen::Matrix<Real, 1, nb_nodes> ShapeFunctionsT;
  typedef Eigen::Matrix<Real, dimensionality, nb_nodes> MappedGradientT;
  typedef Eigen::Matrix<Real, dimensionality, dimension> JacobianT;

  /// Shape function reference
  virtual const ShapeFunction& shape_function() const
  {
    const static SFTriagLagrangeP2 shape_function_obj;
    return shape_function_obj;
  }

  /// Compute the shape functions corresponding to the given
  /// mapped coordinates
  /// @param map_coord The mapped coordinates
  /// @param shapef Vector storing the result
  static void shape_function_value(const MappedCoordsT& map_coord, ShapeFunctionsT& shapef);

  /// Compute Mapped Coordinates
  /// @param coord contains the coordinates to be mapped
  /// @param nodes contains the nodes
  /// @param map_coord Store the output mapped coordinates
  static void mapped_coordinates(const CoordsT& coord, const NodeMatrixT& nodes, MappedCoordsT& map_coord);

  /// Compute the gradient with respect to mapped coordinates, i.e.
  /// partial derivatives are in terms of the mapped coordinates.
  /// The result needs to be multiplied with the inverse jacobian to get the result in real coordinates.
  /// @param map_coord The mapped coordinates where the gradient should be calculated
  /// @param result Storage for the resulting gradient matrix
  static void shape_function_gradient(const MappedCoordsT& map_coord, MappedGradientT& result);

  /// Compute the jacobian determinant at the given mapped coordinates
  static Real jacobian_determinant(const MappedCoordsT& map_coord, const NodeMatrixT& nodes);

  /// Compute the Jacobian matrix
  /// @param map_coord The mapped coordinates where the Jacobian should be calculated
  /// @param result Storage for the resulting Jacobian matrix
  static void jacobian(const MappedCoordsT& map_coord, const NodeMatrixT& nodes, JacobianT& result);

  /// Compute the adjoint of Jacobian matrix
  /// @param map_coord The mapped coordinates where the Jacobian should be calculated
  /// @param result Storage for the resulting adjoint
  static void jacobian_adjoint(const MappedCoordsT& map_coord, const NodeMatrixT& nodes, JacobianT& result);

  /// Volume of the cell
  static Real volume(const NodeMatrixT& nodes);
  
  static bool in_element(const CoordsT& coord, const NodeMatrixT& nodes);

  static const FaceConnectivity& faces();

  virtual Real compute_volume(const NodesT& coord) const;
  virtual bool is_coord_in_element(const RealVector& coord, const NodesT& nodes) const;
  virtual const CF::Mesh::ElementType::FaceConnectivity& face_connectivity() const;
  virtual const CF::Mesh::ElementType& face_type(const CF::Uint face) const;

private:

  /// Helper function for reuse in volume() and jacobian_determinant()
  static Real jacobian_determinant(const NodeMatrixT& nodes);

};

} // SF
} // Mesh
} // CF

#endif /* CF_Mesh_SF_Triag2DLagrangeP2 */
