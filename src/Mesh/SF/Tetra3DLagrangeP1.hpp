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
	
using namespace Math::MathConsts;

/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P1 (linear)
/// triangular element.
/// @author Andrea Lani
/// @author Geoffrey Deliege
/// @author Tiago Quintino
/// @author Bart Janssens
struct MESH_SF_API Tetra3DLagrangeP1  : public Tetra3D
{

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
static void shape_function(const MappedCoordsT& mappedCoord, ShapeFunctionsT& shapeFunc)
{
  shapeFunc[0] = 1.0 - mappedCoord[0] - mappedCoord[1] - mappedCoord[2];
  shapeFunc[1] = mappedCoord[0];
  shapeFunc[2] = mappedCoord[1];
  shapeFunc[3] = mappedCoord[2];
}

/// Compute Mapped Coordinates
/// @param coord contains the coordinates to be mapped
/// @param nodes contains the nodes
/// @param mappedCoord Store the output mapped coordinates
template<typename NodesT>
static void mapped_coordinates(const CoordsT& coord, const NodesT& nodes, MappedCoordsT& mappedCoord)
{
  RealMatrix3 M;
  M.col(0) = nodes.row(1) - nodes.row(0);
  M.col(1) = nodes.row(2) - nodes.row(0);
  M.col(2) = nodes.row(3) - nodes.row(0);
  
  mappedCoord = M.inverse() * (coord - nodes.row(0).transpose());
}

/// Compute the gradient with respect to mapped coordinates, i.e. parial derivatives are in terms of the
/// mapped coordinates. The result needs to be multiplied with the inverse jacobian to get the result in real
/// coordinates.
/// @param mappedCoord The mapped coordinates where the gradient should be calculated
/// @param result Storage for the resulting gradient matrix
static void mapped_gradient(const MappedCoordsT& mappedCoord, MappedGradientT& result)
{
  result(XX, 0) = -1.;
  result(YY, 0) = -1.;
  result(ZZ, 0) = -1.;
  result(XX, 1) = 1.;
  result(YY, 1) = 0.;
  result(ZZ, 1) = 0.;
  result(XX, 2) = 0.;
  result(YY, 2) = 1.;
  result(ZZ, 2) = 0.;
  result(XX, 3) = 0.;
  result(YY, 3) = 0.;
  result(ZZ, 3) = 1.;
}

/// Compute the jacobian determinant at the given mapped coordinates
template<typename NodesT>
static Real jacobian_determinant(const MappedCoordsT& mappedCoord, const NodesT& nodes)
{
  return jacobian_determinant(nodes);
}

/// Compute the Jacobian matrix
/// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
/// @param result Storage for the resulting Jacobian matrix
template<typename NodesT>
static void jacobian(const MappedCoordsT& mappedCoord, const NodesT& nodes, JacobianT& result)
{
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

  const Real dxdksi = -x0 + x1;
  const Real dydksi = -y0 + y1;
  const Real dzdksi = -z0 + z1;

  const Real dxdeta = -x0 + x2;
  const Real dydeta = -y0 + y2;
  const Real dzdeta = -z0 + z2;

  const Real dxdzta = -x0 + x3;
  const Real dydzta = -y0 + y3;
  const Real dzdzta = -z0 + z3;

  // Derivatives of shape functions are constant
  // hence Jacobians are independent of the mappedCoord
  result(KSI,XX) = dxdksi;
  result(KSI,YY) = dydksi;
  result(KSI,ZZ) = dzdksi;

  result(ETA,XX) = dxdeta;
  result(ETA,YY) = dydeta;
  result(ETA,ZZ) = dzdeta;

  result(ZTA,XX) = dxdzta;
  result(ZTA,YY) = dydzta;
  result(ZTA,ZZ) = dzdzta;
}

/// Compute the adjoint of Jacobian matrix
/// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
/// @param result Storage for the resulting adjoint
template<typename NodesT>
static void jacobian_adjoint(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& result)
{
  JacobianT J;
  jacobian(mapped_coord, nodes, J);
  result(0, 0) =  (J(1, 1)*J(2, 2) - J(1, 2)*J(2, 1));
  result(0, 1) = -(J(0, 1)*J(2, 2) - J(0, 2)*J(2, 1));
  result(0, 2) =  (J(0, 1)*J(1, 2) - J(1, 1)*J(0, 2));
  result(1, 0) = -(J(1, 0)*J(2, 2) - J(1, 2)*J(2, 0));
  result(1, 1) =  (J(0, 0)*J(2, 2) - J(0, 2)*J(2, 0));
  result(1, 2) = -(J(0, 0)*J(1, 2) - J(0, 2)*J(1, 0));
  result(2, 0) =  (J(1, 0)*J(2, 1) - J(1, 1)*J(2, 0));
  result(2, 1) = -(J(0, 0)*J(2, 1) - J(0, 1)*J(2, 0));
  result(2, 2) =  (J(0, 0)*J(1, 1) - J(0, 1)*J(1, 0));
}

/// Volume of the cell
template<typename NodesT>
static Real volume(const NodesT& nodes)
{
  return jacobian_determinant(nodes) / 6.;
}

template<typename NodesT>
static bool in_element(const CoordsT& coord, const NodesT& nodes)
{
  MappedCoordsT mapped_coord;
  mapped_coordinates(coord, nodes, mapped_coord);
  if((mapped_coord[KSI] >= -eps()) &&
    (mapped_coord[ETA] >= -eps()) &&
    (mapped_coord[ZTA] >= -eps()) &&
    (mapped_coord.sum() <= 1.))
  {
    return true;
  }
  else
  {
    return false;
  }
}

static const FaceConnectivity& faces();

Tetra3DLagrangeP1();
virtual std::string getElementTypeName() const;
virtual Real computeVolume(const NodesT& coord) const;
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

} // namespace SF
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_SF_Tetra3DLagrangeP1 */
