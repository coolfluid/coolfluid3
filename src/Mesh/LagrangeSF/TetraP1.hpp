#ifndef CF_Mesh_LagrangeSF_TetraP1_hpp
#define CF_Mesh_LagrangeSF_TetraP1_hpp

#include "Common/AssertionManager.hpp"

#include "Math/MatrixInverterT.hpp"
#include "Math/RealMatrix.hpp"

#include "Mesh/GeoShape.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeSF {

/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P1 (linear)
/// tetrahedral element.
/// @author Andrea Lani
/// @author Geoffrey Deliege
/// @author Tiago Quintino
/// @author Bart Janssens
class TetraP1 {
public:

/// Shape represented by this shape function
static const GeoShape::Type shape = GeoShape::TETRA;
/// Order of the shape function
static const Uint order = 1;
/// Dimensionality of the shape function
static const Uint dimensions = 3;

/// Compute the shape functions corresponding to the given
/// mapped coordinates
/// @param mappedCoord The mapped coordinates
/// @param shapeFunc Vector storing the result
static void computeShapeFunction(const RealVector& mappedCoord, RealVector& shapeFunc) {
  cf_assert(shapeFunc.size() == 4);
  cf_assert(mappedCoord.size() == 3);
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
static void computeMappedCoordinates(const RealVector& coord, const NodesT& nodes, RealVector& mappedCoord) {
  cf_assert(coord.size() == 3);
  cf_assert(mappedCoord.size() == 3);
  cf_assert(nodes.size() == 4);

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
static void computeMappedGradient(const RealVector& mappedCoord, RealMatrix& result) {
  cf_assert(result.nbRows() == 4);
  cf_assert(result.nbCols() == 3);
  result(0, XX) = -1.;
  result(0, YY) = -1.;
  result(0, ZZ) = -1.;
  result(1, XX) = 1.;
  result(1, YY) = 0.;
  result(1, ZZ) = 0.;
  result(2, XX) = 0.;
  result(2, YY) = 1.;
  result(2, ZZ) = 0.;
  result(3, XX) = 0.;
  result(3, YY) = 0.;
  result(3, ZZ) = 1.;
}

/// Compute the jacobian determinant at the given mapped coordinates
template<typename NodesT>
inline static Real computeJacobianDeterminant(const RealVector& mappedCoord, const NodesT& nodes) {
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

/// Compute the Jacobian matrix
/// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
/// @param result Storage for the resulting Jacobian matrix
template<typename NodesT>
static void computeJacobian(const RealVector& mappedCoord, const NodesT& nodes, RealMatrix& result) {
  cf_assert(result.nbRows() == 3);
  cf_assert(result.isSquare());

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
static void computeJacobianAdjoint(const RealVector& mappedCoord, const NodesT& nodes, RealMatrix& result) {
  cf_assert(result.nbRows() == 3);
  cf_assert(result.isSquare());
  RealMatrix jacobian(3,3);
  computeJacobian(mappedCoord, nodes, jacobian);

  result[0] =  (jacobian[4]*jacobian[8] - jacobian[5]*jacobian[7]);
  result[1] = -(jacobian[1]*jacobian[8] - jacobian[2]*jacobian[7]);
  result[2] =  (jacobian[1]*jacobian[5] - jacobian[4]*jacobian[2]);
  result[3] = -(jacobian[3]*jacobian[8] - jacobian[5]*jacobian[6]);
  result[4] =  (jacobian[0]*jacobian[8] - jacobian[2]*jacobian[6]);
  result[5] = -(jacobian[0]*jacobian[5] - jacobian[2]*jacobian[3]);
  result[6] =  (jacobian[3]*jacobian[7] - jacobian[4]*jacobian[6]);
  result[7] = -(jacobian[0]*jacobian[7] - jacobian[1]*jacobian[6]);
  result[8] =  (jacobian[0]*jacobian[4] - jacobian[1]*jacobian[3]);
}

};

} // namespace LagrangeSF
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_LagrangeSF_TetraP1 */
