// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SF_Line1DLagrangeP1_hpp
#define CF_Mesh_SF_Line1DLagrangeP1_hpp

#include "Math/RealMatrix.hpp"

#include "Mesh/Line1D.hpp"

#include "Mesh/SF/LibSF.hpp"

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
struct SF_API Line1DLagrangeP1  : public Line1D {

/// Compute the shape functions corresponding to the given
/// mapped coordinates
/// @param mappedCoord The mapped coordinates
/// @param shapeFunc Vector storing the result
static void shape_function(const RealVector& mappedCoord, RealVector& shapeFunc)
{
  cf_assert(shapeFunc.size() == nb_nodes);
  cf_assert(mappedCoord.size() == dimension);

  shapeFunc[0] = 0.5 * (1.0 - mappedCoord[KSI]);
  shapeFunc[1] = 0.5 * (1.0 + mappedCoord[KSI]);
}

/// Compute Mapped Coordinates
/// @param coord contains the coordinates to be mapped
/// @param nodes contains the nodes
/// @param mappedCoord Store the output mapped coordinates
template<typename NodesT>
static void mapped_coordinates(const RealVector& coord, const NodesT& nodes, RealVector& mappedCoord)
{
  cf_assert(coord.size() == dimension);
  cf_assert(mappedCoord.size() == dimension);
  cf_assert(nodes.size() == nb_nodes);

  const Real x0 = nodes[0][XX];
  const Real x1 = nodes[1][XX];
  mappedCoord[KSI] = (2*coord[0] - (x1 + x0)) / (x1 - x0);
}

/// Compute the gradient with respect to mapped coordinates, i.e. parial derivatives are in terms of the
/// mapped coordinates. The result needs to be multiplied with the inverse jacobian to get the result in real
/// coordinates.
/// @param mappedCoord The mapped coordinates where the gradient should be calculated
/// @param result Storage for the resulting gradient matrix
static void mapped_gradient(const RealVector& mappedCoord, RealMatrix& result)
{
  cf_assert(result.nbCols() == nb_nodes);
  cf_assert(result.nbRows() == dimension);
  result(XX, 0) = -0.5;
  result(XX, 1) = 0.5;
}

/// Compute the jacobian determinant at the given mapped coordinates
template<typename NodesT>
static Real jacobian_determinant(const RealVector& mappedCoord, const NodesT& nodes) {
  return 0.5*(nodes[1][XX] - nodes[0][XX]);
}

/// Compute the Jacobian matrix
/// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
/// @param result Storage for the resulting Jacobian matrix
template<typename NodesT>
static void jacobian(const RealVector& mappedCoord, const NodesT& nodes, RealMatrix& result) {
  cf_assert(result.nbRows() == dimension);
  cf_assert(result.isSquare());
  result(KSI,XX) = jacobian_determinant(mappedCoord, nodes);
}

/// Compute the adjoint of Jacobian matrix
/// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
/// @param result Storage for the resulting adjoint
template<typename NodesT>
static void jacobian_adjoint(const RealVector& mappedCoord, const NodesT& nodes, RealMatrix& result) {
  cf_assert(result.nbRows() == dimension);
  cf_assert(result.isSquare());
  result(KSI,XX) = 1.;
}

/// Volume of the cell
template<typename NodesT>
static Real volume(const NodesT& nodes) {
  return std::abs(nodes[1][XX] - nodes[0][XX]);
}

/// Number of nodes
static const Uint nb_nodes = 2;

/// Order of the shape function
static const Uint order = 1;

Line1DLagrangeP1();
virtual std::string getElementTypeName() const;
virtual Real computeVolume(const NodesT& coord) const;
virtual bool is_coord_in_element(const RealVector& coord, const NodesT& nodes) const;
virtual const CF::Mesh::ElementType::FaceConnectivity& face_connectivity() const;
virtual const CF::Mesh::ElementType& face_type(const CF::Uint face) const;

};

} // namespace SF
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_SF_Line1DLagrangeP1 */
