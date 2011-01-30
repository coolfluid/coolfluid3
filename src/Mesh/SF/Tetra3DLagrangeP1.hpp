// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SF_Tetra3DLagrangeP1_hpp
#define CF_Mesh_SF_Tetra3DLagrangeP1_hpp

#include "Math/MatrixTypes.hpp"
#include "Math/MathConsts.hpp"

#include "Mesh/GeoShape.hpp"
#include "Mesh/Tetra3D.hpp"

#include "Mesh/SF/LibSF.hpp"

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
struct MESH_SF_API Tetra3DLagrangeP1  : public Tetra3D
{

  Tetra3DLagrangeP1(const std::string& name = type_name());

  static std::string type_name() { return "Tetra3DLagrangeP1"; }

  virtual std::string builder_name() const { return LibSF::library_namespace()+"."+type_name(); }

/// Number of nodes
static const Uint nb_nodes = 4;

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

/// Compute the jacobian determinant at the given mapped coordinates
static Real jacobian_determinant(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes);

/// Compute the Jacobian matrix
/// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
/// @param result Storage for the resulting Jacobian matrix
static void jacobian(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, JacobianT& result);

/// Compute the adjoint of Jacobian matrix
/// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
/// @param result Storage for the resulting adjoint
static void jacobian_adjoint(const MappedCoordsT& mapped_coord, const NodeMatrixT& nodes, JacobianT& result);

/// Volume of the cell
static Real volume(const NodeMatrixT& nodes);

static bool in_element(const CoordsT& coord, const NodeMatrixT& nodes);

static const FaceConnectivity& faces();

virtual std::string element_type_name() const;
virtual Real compute_volume(const NodesT& coord) const;
virtual bool is_coord_in_element(const RealVector& coord, const NodesT& nodes) const;
virtual const CF::Mesh::ElementType::FaceConnectivity& face_connectivity() const;
virtual const CF::Mesh::ElementType& face_type(const CF::Uint face) const;

private:

/// Helper function for reuse in volume() and jacobian_determinant()
template<typename NodesT>
static Real jacobian_determinant(const NodesT& nodes) {
  const Real x0 = nodes(0, XX);
  const Real y0 = nodes(0, YY);
  const Real z0 = nodes(0, ZZ);

  const Real x1 = nodes(1, XX);
  const Real y1 = nodes(1, YY);
  const Real z1 = nodes(1, ZZ);

  const Real x2 = nodes(2, XX);
  const Real y2 = nodes(2, YY);
  const Real z2 = nodes(2, ZZ);

  const Real x3 = nodes(3, XX);
  const Real y3 = nodes(3, YY);
  const Real z3 = nodes(3, ZZ);

  return
      x2*y1*z0 - x3*y1*z0 - x1*y2*z0 + x3*y2*z0 + x1*y3*z0 -
      x2*y3*z0 - x2*y0*z1 + x3*y0*z1 + x0*y2*z1 - x3*y2*z1 -
      x0*y3*z1 + x2*y3*z1 + x1*y0*z2 - x3*y0*z2 - x0*y1*z2 +
      x3*y1*z2 + x0*y3*z2 - x1*y3*z2 - x1*y0*z3 + x2*y0*z3 +
      x0*y1*z3 - x2*y1*z3 - x0*y2*z3 + x1*y2*z3;
}

};

} // SF
} // Mesh
} // CF

#endif /* CF_Mesh_SF_Tetra3DLagrangeP1 */
