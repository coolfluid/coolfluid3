// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SF_Point1DLagrangeP1_hpp
#define CF_Mesh_SF_Point1DLagrangeP1_hpp

#include "Math/MatrixTypes.hpp"

#include "Mesh/Point1D.hpp"

#include "Mesh/SF/LibSF.hpp"
#include "Mesh/ElementData.hpp"

namespace CF {
namespace Mesh {
namespace SF {

/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P1 (linear)
/// line element.
/// @author Andrea Lani
/// @author Geoffrey Deliege
/// @author Tiago Quintino
/// @author Bart Janssens
struct MESH_SF_API Point1DLagrangeP1  : public Point1D {
  
  Point1DLagrangeP1(const std::string& name = type_name());

  static std::string type_name() { return "Point1DLagrangeP1"; }

/// Number of nodes
static const Uint nb_nodes = 1;

/// Order of the shape function
static const Uint order = 1;
  
/// Types for the matrices used
typedef Eigen::Matrix<Real, dimension, 1> CoordsT;
typedef Eigen::Matrix<Real, dimensionality, 1> MappedCoordsT;
typedef Eigen::Matrix<Real, nb_nodes, dimension> NodeMatrixT;
typedef Eigen::Matrix<Real, 1, nb_nodes> ShapeFunctionsT;
typedef Eigen::Matrix<Real, dimensionality, nb_nodes> MappedGradientT;
typedef Eigen::Matrix<Real, dimensionality, dimension> JacobianT;

/// Compute the shape functions corresponding to the given
/// mapped coordinates
/// @param mappedCoord The mapped coordinates
/// @param shapeFunc Vector storing the result
static void shape_function(const MappedCoordsT& mappedCoord, ShapeFunctionsT& shapeFunc);

/// Compute Mapped Coordinates
/// @param coord contains the coordinates to be mapped
/// @param nodes contains the nodes
/// @param mappedCoord Store the output mapped coordinates
static void mapped_coordinates(const CoordsT& coord, const NodeMatrixT& nodes, MappedCoordsT& mappedCoord);

/// Compute the gradient with respect to mapped coordinates, i.e. parial derivatives are in terms of the
/// mapped coordinates. The result needs to be multiplied with the inverse jacobian to get the result in real
/// coordinates.
/// @param mappedCoord The mapped coordinates where the gradient should be calculated
/// @param result Storage for the resulting gradient matrix
static void mapped_gradient(const MappedCoordsT& mappedCoord, MappedGradientT& result);

/// Compute the Jacobian matrix
/// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
/// @param result Storage for the resulting Jacobian matrix
static void jacobian(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, JacobianT& result);

/// Normal vector to the surface. Length equals the jacobian norm.
/// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
/// @param result Storage for the resulting Jacobian matrix
static void normal(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, CoordsT& result);

/// The area of an element that represents a surface in the solution space, i.e.
/// 1D elements in 2D space or 2D elements in 3D space
static Real area(const NodeMatrixT& nodes);

/// Volume of the cell
static Real volume(const NodeMatrixT& nodes);

virtual std::string element_type_name() const;
virtual Real compute_volume(const NodesT& coord) const;
virtual bool is_coord_in_element( const RealVector& coord, const NodesT& nodes) const;
virtual const CF::Mesh::ElementType::FaceConnectivity& face_connectivity() const;
virtual const CF::Mesh::ElementType& face_type(const CF::Uint face) const;

};

} // SF
} // Mesh
} // CF

#endif /* CF_Mesh_SF_Point1DLagrangeP1 */
