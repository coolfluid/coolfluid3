// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SF_Tetra3DLagrangeP1_hpp
#define CF_Mesh_SF_Tetra3DLagrangeP1_hpp

#include "Math/RealMatrix.hpp"
#include "Math/MathConsts.hpp"
#include "Math/MatrixInverterT.hpp"

#include "Mesh/GeoShape.hpp"
#include "Mesh/Tetra3D.hpp"

#include "Mesh/SF/SFLib.hpp"

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
struct SF_API Tetra3DLagrangeP1  : public Tetra3D
{

/// Compute the shape functions corresponding to the given
/// mapped coordinates
/// @param mappedCoord The mapped coordinates
/// @param shapeFunc Vector storing the result
static void shape_function(const RealVector& mappedCoord, RealVector& shapeFunc)
{
  cf_assert(shapeFunc.size() == nb_nodes);
  cf_assert(mappedCoord.size() == dimension);

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
static void mapped_coordinates(const RealVector& coord, const NodesT& nodes, RealVector& mappedCoord)
{
  cf_assert(coord.size() == dimension);
  cf_assert(mappedCoord.size() == dimension);
  cf_assert(nodes.size() == nb_nodes);

  const RealVector& xA = nodes[0];
  const RealVector& xB = nodes[1];
  const RealVector& xC = nodes[2];
  const RealVector& xD = nodes[3];

  RealVector vec1 = xB-xA;
  RealVector vec2 = xC-xA;
  RealVector vec3 = xD-xA;
  RealVector vec4 = coord-xA;

  RealMatrix matrix1(3,3);

  matrix1(0,0) = vec1[XX];
  matrix1(0,1) = vec2[XX];
  matrix1(0,2) = vec3[XX];
  matrix1(1,0) = vec1[YY];
  matrix1(1,1) = vec2[YY];
  matrix1(1,2) = vec3[YY];
  matrix1(2,0) = vec1[ZZ];
  matrix1(2,1) = vec2[ZZ];
  matrix1(2,2) = vec3[ZZ];

  RealMatrix matrix2(3,3);
  Math::MatrixInverterT<3> inverter;
  inverter.invert(matrix1,matrix2);

  mappedCoord = matrix2 * vec4;
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
static Real jacobian_determinant(const RealVector& mappedCoord, const NodesT& nodes)
{
  return jacobian_determinant(nodes);
}

/// Compute the Jacobian matrix
/// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
/// @param result Storage for the resulting Jacobian matrix
template<typename NodesT>
static void jacobian(const RealVector& mappedCoord, const NodesT& nodes, RealMatrix& result) {
  cf_assert(result.nbRows() == dimensionality);
  cf_assert(result.nbCols() == dimension);
  const Real x0 = nodes[0][XX];
  const Real y0 = nodes[0][YY];
  const Real z0 = nodes[0][ZZ];

  const Real x1 = nodes[1][XX];
  const Real y1 = nodes[1][YY];
  const Real z1 = nodes[1][ZZ];

  const Real x2 = nodes[2][XX];
  const Real y2 = nodes[2][YY];
  const Real z2 = nodes[2][ZZ];

  const Real x3 = nodes[3][XX];
  const Real y3 = nodes[3][YY];
  const Real z3 = nodes[3][ZZ];

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
static void jacobian_adjoint(const RealVector& mappedCoord, const NodesT& nodes, RealMatrix& result) {
  cf_assert(result.nbRows() == dimensionality);
  cf_assert(result.nbCols() == dimension);
  RealMatrix jac(dimensionality,dimension);
  jacobian(mappedCoord, nodes, jac);

  result[0] =  (jac[4]*jac[8] - jac[5]*jac[7]);
  result[1] = -(jac[1]*jac[8] - jac[2]*jac[7]);
  result[2] =  (jac[1]*jac[5] - jac[4]*jac[2]);
  result[3] = -(jac[3]*jac[8] - jac[5]*jac[6]);
  result[4] =  (jac[0]*jac[8] - jac[2]*jac[6]);
  result[5] = -(jac[0]*jac[5] - jac[2]*jac[3]);
  result[6] =  (jac[3]*jac[7] - jac[4]*jac[6]);
  result[7] = -(jac[0]*jac[7] - jac[1]*jac[6]);
  result[8] =  (jac[0]*jac[4] - jac[1]*jac[3]);
}

/// Volume of the cell
template<typename NodesT>
static Real volume(const NodesT& nodes) {
  return jacobian_determinant(nodes) / 6.;
}

template<typename NodesT>
static bool in_element(const RealVector& coord, const NodesT& nodes)
{
	RealVector mapped_coord(coord.size());
	mapped_coordinates(coord, nodes, mapped_coord);
	if((mapped_coord[KSI] >= -Math::MathConsts::RealEps()) &&
		 (mapped_coord[ETA] >= -Math::MathConsts::RealEps()) &&
		 (mapped_coord[ZTA] >= -Math::MathConsts::RealEps()) &&
		 (mapped_coord.sum() <= 1.))
	{
		return true;
	}
	else
	{
		return false;
	}
}


/// Number of nodes
static const Uint nb_nodes = 4;

/// Order of the shape function
static const Uint order = 1;

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
  const Real x0 = nodes[0][XX];
  const Real y0 = nodes[0][YY];
  const Real z0 = nodes[0][ZZ];

  const Real x1 = nodes[1][XX];
  const Real y1 = nodes[1][YY];
  const Real z1 = nodes[1][ZZ];

  const Real x2 = nodes[2][XX];
  const Real y2 = nodes[2][YY];
  const Real z2 = nodes[2][ZZ];

  const Real x3 = nodes[3][XX];
  const Real y3 = nodes[3][YY];
  const Real z3 = nodes[3][ZZ];

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
