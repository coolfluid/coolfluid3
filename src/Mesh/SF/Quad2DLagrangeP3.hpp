
// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SF_Quad2DLagrangeP3_hpp
#define CF_Mesh_SF_Quad2DLagrangeP3_hpp

#include "Math/MatrixTypes.hpp"

#include "Mesh/Quad2D.hpp"
#include "Mesh/SF/LibSF.hpp"

namespace CF {
namespace Mesh {
namespace SF {

/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P1 (bilinear)
/// quadrilateral element.
/// @author Andrea Lani
/// @author Geoffrey Deliege
/// @author Tiago Quintino
/// @author Bart Janssens
/// @author Willem Deconinck
struct MESH_SF_API Quad2DLagrangeP3  : public Quad2D {
  

  Quad2DLagrangeP3(const std::string& name = type_name());

  static std::string type_name() { return "Quad2DLagrangeP3"; }

  virtual std::string builder_name() const { return LibSF::library_namespace()+"."+type_name(); }

/// Number of nodes
static const Uint nb_nodes = 16;

/// Order of the shape function
static const Uint order = 3;

/// Types for the matrices used
typedef Eigen::Matrix<Real, dimension, 1> CoordsT;
typedef Eigen::Matrix<Real, dimensionality, 1> MappedCoordsT;
typedef Eigen::Matrix<Real, nb_nodes, dimension> NodeMatrixT;
typedef Eigen::Matrix<Real, 1, nb_nodes> ShapeFunctionsT;
typedef Eigen::Matrix<Real, dimensionality, nb_nodes> MappedGradientT;
typedef Eigen::Matrix<Real, dimensionality, dimension> JacobianT;

/// typedef for the supporting geometry
typedef Quad2D Support;

/// Compute the shape functions corresponding to the given
/// mapped coordinates
/// @param mappedCoord The mapped coordinates
/// @param shapeFunc Vector storing the result
static void shape_function_value(const MappedCoordsT& mappedCoord, ShapeFunctionsT& shapeFunc);

/// Compute Mapped Coordinates
/// @param coord contains the coordinates to be mapped
/// @param nodes contains the nodes
/// @param mappedCoord Store the output mapped coordinates
static void mapped_coordinates(const CoordsT& coord, const NodeMatrixT& nodes, MappedCoordsT& mappedCoord);

/// Compute the gradient with respect to mapped coordinates, i.e. partial derivatives are in terms of the
/// mapped coordinates. The result needs to be multiplied with the inverse jacobian to get the result in real
/// coordinates.
/// @param mappedCoord The mapped coordinates where the gradient should be calculated
/// @param result Storage for the resulting gradient matrix
static void shape_function_gradient(const MappedCoordsT& mappedCoord, MappedGradientT& result);

/// Compute the jacobian determinant at the given mapped coordinates
static Real jacobian_determinant(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes);

/// Compute the Jacobian matrix
/// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
/// @param result Storage for the resulting Jacobian matrix
static void jacobian(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, JacobianT& result);

/// Compute the adjoint of Jacobian matrix
/// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
/// @param result Storage for the resulting adjoint
static void jacobian_adjoint(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, JacobianT& result);

/// Volume of the cell
static Real volume(const NodeMatrixT& nodes);

/// Connectivity info for the faces
static const FaceConnectivity& faces();

virtual Real compute_volume(const NodesT& coord) const;
virtual bool is_coord_in_element(const RealVector& coord, const NodesT& nodes) const;
virtual const CF::Mesh::ElementType::FaceConnectivity& face_connectivity() const;
virtual const CF::Mesh::ElementType& face_type(const CF::Uint face) const;

};

} // SF
} // Mesh
} // CF

#endif /* CF_Mesh_SF_Quad2DLagrangeP3 */
