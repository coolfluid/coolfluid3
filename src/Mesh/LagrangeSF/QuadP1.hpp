#ifndef CF_Mesh_LagrangeSF_QuadP1_HH
#define CF_Mesh_LagrangeSF_QuadP1_HH

#include "Common/AssertionManager.hpp"
#include "Math/MatrixInverterT.hpp"
#include "Mesh/LagrangeSF/LagrangeSF.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeSF {

/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P1 (bilinear)
/// quadrilateral element.
/// @author Andrea Lani
/// @author Geoffrey Deliege
/// @author Tiago Quintino
/// @author Bart Janssens
class QuadP1 {
public:

/// Compute the shape functions corresponding to the given
/// mapped coordinates
/// @param mappedCoord The mapped coordinates
/// @param shapeFunc Vector storing the result
static void computeShapeFunction(const RealVector& mappedCoord, RealVector& shapeFunc) {
  cf_assert(shapeFunc.size() == 4);
  cf_assert(mappedCoord.size() == 2);
  const Real xi  = mappedCoord[KSI];
  const Real eta = mappedCoord[ETA];

  shapeFunc[0] = 0.25 * (1.0 - xi) * (1.0 - eta);
  shapeFunc[1] = 0.25 * (1.0 + xi) * (1.0 - eta);
  shapeFunc[2] = 0.25 * (1.0 + xi) * (1.0 + eta);
  shapeFunc[3] = 0.25 * (1.0 - xi) * (1.0 + eta);
}

/// Compute the jacobian determinant at the given
/// mapped coordinates
inline static Real computeJacobianDeterminant(const RealVector& mappedCoord, const NodesT& nodes) {
  cf_assert(mappedCoord.size() == 2);
  cf_assert(nodes.size() == 4);
  JacobianCoefficients jc(nodes);
  const Real xi  = mappedCoord[0];
  const Real eta = mappedCoord[1];
  return (jc.bx*jc.dy - jc.by*jc.dx)*eta + (jc.dx*jc.cy - jc.cx*jc.dy)*xi + jc.bx*jc.cy - jc.by*jc.cx;
}

/// Compute Mapped Coordinates
/// @param coord contains the coordinates to be mapped
/// @param nodes contains the nodes
/// @param mappedCoord Store the output mapped coordinates
static void computeMappedCoordinates(const RealVector& coord, const NodesT& nodes, RealVector& mappedCoord) {
  cf_assert(coord.size() == 2);
  cf_assert(mappedCoord.size() == 2);
  cf_assert(nodes.size() == 4);

  const Real x = coord[XX];
  const Real y = coord[YY];
  JacobianCoefficients jc(nodes);
  mappedCoord[KSI] = (jc.ax*jc.dy + jc.bx*jc.cy + jc.dx*y - jc.ay*jc.dx - jc.by*jc.cx - jc.dy*x - sqrt(-4*jc.bx*jc.cx*jc.dy*y - 4*jc.by*jc.cy*jc.dx*x - 2*jc.ax*jc.ay*jc.dx*jc.dy - 2*jc.ax*jc.bx*jc.cy*jc.dy - 2*jc.ax*jc.by*jc.cx*jc.dy - 2*jc.ay*jc.bx*jc.cy*jc.dx - 2*jc.ay*jc.by*jc.cx*jc.dx - 2*jc.bx*jc.by*jc.cx*jc.cy - 2*jc.dx*jc.dy*x*y + 2*jc.ax*jc.dx*jc.dy*y + 2*jc.ay*jc.dx*jc.dy*x + 2*jc.bx*jc.cy*jc.dx*y + 2*jc.bx*jc.cy*jc.dy*x + 2*jc.by*jc.cx*jc.dx*y + 2*jc.by*jc.cx*jc.dy*x + 4*jc.ax*jc.by*jc.cy*jc.dx + 4*jc.ay*jc.bx*jc.cx*jc.dy + jc.ax*jc.ax*jc.dy*jc.dy + jc.ay*jc.ay*jc.dx*jc.dx + jc.bx*jc.bx*jc.cy*jc.cy + jc.by*jc.by*jc.cx*jc.cx + jc.dx*jc.dx*y*y + jc.dy*jc.dy*x*x - 2*jc.ax*x*jc.dy*jc.dy - 2*jc.ay*y*jc.dx*jc.dx))/(-2*jc.bx*jc.dy + 2*jc.by*jc.dx);
  mappedCoord[ETA] = (x - jc.ax - jc.bx*(jc.ax*jc.dy + jc.bx*jc.cy + jc.dx*y - jc.ay*jc.dx - jc.by*jc.cx - jc.dy*x - sqrt(-4*jc.bx*jc.cx*jc.dy*y - 4*jc.by*jc.cy*jc.dx*x - 2*jc.ax*jc.ay*jc.dx*jc.dy - 2*jc.ax*jc.bx*jc.cy*jc.dy - 2*jc.ax*jc.by*jc.cx*jc.dy - 2*jc.ay*jc.bx*jc.cy*jc.dx - 2*jc.ay*jc.by*jc.cx*jc.dx - 2*jc.bx*jc.by*jc.cx*jc.cy - 2*jc.dx*jc.dy*x*y + 2*jc.ax*jc.dx*jc.dy*y + 2*jc.ay*jc.dx*jc.dy*x + 2*jc.bx*jc.cy*jc.dx*y + 2*jc.bx*jc.cy*jc.dy*x + 2*jc.by*jc.cx*jc.dx*y + 2*jc.by*jc.cx*jc.dy*x + 4*jc.ax*jc.by*jc.cy*jc.dx + 4*jc.ay*jc.bx*jc.cx*jc.dy + jc.ax*jc.ax*jc.dy*jc.dy + jc.ay*jc.ay*jc.dx*jc.dx + jc.bx*jc.bx*jc.cy*jc.cy + jc.by*jc.by*jc.cx*jc.cx + jc.dx*jc.dx*y*y + jc.dy*jc.dy*x*x - 2*jc.ax*x*jc.dy*jc.dy - 2*jc.ay*y*jc.dx*jc.dx))/(-2*jc.bx*jc.dy + 2*jc.by*jc.dx))/(jc.cx + jc.dx*(jc.ax*jc.dy + jc.bx*jc.cy + jc.dx*y - jc.ay*jc.dx - jc.by*jc.cx - jc.dy*x - sqrt(-4*jc.bx*jc.cx*jc.dy*y - 4*jc.by*jc.cy*jc.dx*x - 2*jc.ax*jc.ay*jc.dx*jc.dy - 2*jc.ax*jc.bx*jc.cy*jc.dy - 2*jc.ax*jc.by*jc.cx*jc.dy - 2*jc.ay*jc.bx*jc.cy*jc.dx - 2*jc.ay*jc.by*jc.cx*jc.dx - 2*jc.bx*jc.by*jc.cx*jc.cy - 2*jc.dx*jc.dy*x*y + 2*jc.ax*jc.dx*jc.dy*y + 2*jc.ay*jc.dx*jc.dy*x + 2*jc.bx*jc.cy*jc.dx*y + 2*jc.bx*jc.cy*jc.dy*x + 2*jc.by*jc.cx*jc.dx*y + 2*jc.by*jc.cx*jc.dy*x + 4*jc.ax*jc.by*jc.cy*jc.dx + 4*jc.ay*jc.bx*jc.cx*jc.dy + jc.ax*jc.ax*jc.dy*jc.dy + jc.ay*jc.ay*jc.dx*jc.dx + jc.bx*jc.bx*jc.cy*jc.cy + jc.by*jc.by*jc.cx*jc.cx + jc.dx*jc.dx*y*y + jc.dy*jc.dy*x*x - 2*jc.ax*x*jc.dy*jc.dy - 2*jc.ay*y*jc.dx*jc.dx))/(-2*jc.bx*jc.dy + 2*jc.by*jc.dx));
}



private:
/// Cannot be instantiated
QuadP1() {}

/// Cannot be destroyed
~QuadP1() {}

/// Store Jacobian coefficients calculated from the node positions
struct JacobianCoefficients
{
  const Real ax, bx, cx, dx;
  const Real ay, by, cy, dy;
  JacobianCoefficients(const NodesT& nodes) :
    ax(0.25*( nodes[0][XX] + nodes[1][XX] + nodes[2][XX] + nodes[3][XX])),
    bx(0.25*(-nodes[0][XX] + nodes[1][XX] + nodes[2][XX] - nodes[3][XX])),
    cx(0.25*(-nodes[0][XX] - nodes[1][XX] + nodes[2][XX] + nodes[3][XX])),
    dx(0.25*( nodes[0][XX] - nodes[1][XX] + nodes[2][XX] - nodes[3][XX])),
    ay(0.25*( nodes[0][YY] + nodes[1][YY] + nodes[2][YY] + nodes[3][YY])),
    by(0.25*(-nodes[0][YY] + nodes[1][YY] + nodes[2][YY] - nodes[3][YY])),
    cy(0.25*(-nodes[0][YY] - nodes[1][YY] + nodes[2][YY] + nodes[3][YY])),
    dy(0.25*( nodes[0][YY] - nodes[1][YY] + nodes[2][YY] - nodes[3][YY]))
  {}
};

};

} // namespace LagrangeSF
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_LagrangeSF_QuadP1 */
