#ifndef CF_Mesh_SF_Quad2DLagrangeP1_hpp
#define CF_Mesh_SF_Quad2DLagrangeP1_hpp

#include "Common/CF.hpp"
#include "Math/RealMatrix.hpp"
#include "Mesh/GeoShape.hpp"

#include "Mesh/Elements/Quad2D.hpp"

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
struct Quad2DLagrangeP1  : public Quad2D {

/// Compute the shape functions corresponding to the given
/// mapped coordinates
/// @param mappedCoord The mapped coordinates
/// @param shapeFunc Vector storing the result
static void shape_function(const RealVector& mappedCoord, RealVector& shapeFunc)
{
  cf_assert(shapeFunc.size() == 4);
  cf_assert(mappedCoord.size() == 2);
  const Real xi  = mappedCoord[KSI];
  const Real eta = mappedCoord[ETA];

  shapeFunc[0] = 0.25 * (1.0 - xi) * (1.0 - eta);
  shapeFunc[1] = 0.25 * (1.0 + xi) * (1.0 - eta);
  shapeFunc[2] = 0.25 * (1.0 + xi) * (1.0 + eta);
  shapeFunc[3] = 0.25 * (1.0 - xi) * (1.0 + eta);
}

/// Compute Mapped Coordinates
/// @param coord contains the coordinates to be mapped
/// @param nodes contains the nodes
/// @param mappedCoord Store the output mapped coordinates
template<typename NodesT>
static void mapped_coordinates(const RealVector& coord, const NodesT& nodes, RealVector& mappedCoord)
{
  cf_assert(coord.size() == 2);
  cf_assert(mappedCoord.size() == 2);
  cf_assert(nodes.size() == 4);

  const Real x = coord[XX];
  const Real y = coord[YY];
  JacobianCoefficients jc(nodes);
  mappedCoord[KSI] = (jc.ax*jc.dy + jc.bx*jc.cy + jc.dx*y - jc.ay*jc.dx - jc.by*jc.cx - jc.dy*x - sqrt(-4*jc.bx*jc.cx*jc.dy*y - 4*jc.by*jc.cy*jc.dx*x - 2*jc.ax*jc.ay*jc.dx*jc.dy - 2*jc.ax*jc.bx*jc.cy*jc.dy - 2*jc.ax*jc.by*jc.cx*jc.dy - 2*jc.ay*jc.bx*jc.cy*jc.dx - 2*jc.ay*jc.by*jc.cx*jc.dx - 2*jc.bx*jc.by*jc.cx*jc.cy - 2*jc.dx*jc.dy*x*y + 2*jc.ax*jc.dx*jc.dy*y + 2*jc.ay*jc.dx*jc.dy*x + 2*jc.bx*jc.cy*jc.dx*y + 2*jc.bx*jc.cy*jc.dy*x + 2*jc.by*jc.cx*jc.dx*y + 2*jc.by*jc.cx*jc.dy*x + 4*jc.ax*jc.by*jc.cy*jc.dx + 4*jc.ay*jc.bx*jc.cx*jc.dy + jc.ax*jc.ax*jc.dy*jc.dy + jc.ay*jc.ay*jc.dx*jc.dx + jc.bx*jc.bx*jc.cy*jc.cy + jc.by*jc.by*jc.cx*jc.cx + jc.dx*jc.dx*y*y + jc.dy*jc.dy*x*x - 2*jc.ax*x*jc.dy*jc.dy - 2*jc.ay*y*jc.dx*jc.dx))/(-2*jc.bx*jc.dy + 2*jc.by*jc.dx);
  mappedCoord[ETA] = (x - jc.ax - jc.bx*(jc.ax*jc.dy + jc.bx*jc.cy + jc.dx*y - jc.ay*jc.dx - jc.by*jc.cx - jc.dy*x - sqrt(-4*jc.bx*jc.cx*jc.dy*y - 4*jc.by*jc.cy*jc.dx*x - 2*jc.ax*jc.ay*jc.dx*jc.dy - 2*jc.ax*jc.bx*jc.cy*jc.dy - 2*jc.ax*jc.by*jc.cx*jc.dy - 2*jc.ay*jc.bx*jc.cy*jc.dx - 2*jc.ay*jc.by*jc.cx*jc.dx - 2*jc.bx*jc.by*jc.cx*jc.cy - 2*jc.dx*jc.dy*x*y + 2*jc.ax*jc.dx*jc.dy*y + 2*jc.ay*jc.dx*jc.dy*x + 2*jc.bx*jc.cy*jc.dx*y + 2*jc.bx*jc.cy*jc.dy*x + 2*jc.by*jc.cx*jc.dx*y + 2*jc.by*jc.cx*jc.dy*x + 4*jc.ax*jc.by*jc.cy*jc.dx + 4*jc.ay*jc.bx*jc.cx*jc.dy + jc.ax*jc.ax*jc.dy*jc.dy + jc.ay*jc.ay*jc.dx*jc.dx + jc.bx*jc.bx*jc.cy*jc.cy + jc.by*jc.by*jc.cx*jc.cx + jc.dx*jc.dx*y*y + jc.dy*jc.dy*x*x - 2*jc.ax*x*jc.dy*jc.dy - 2*jc.ay*y*jc.dx*jc.dx))/(-2*jc.bx*jc.dy + 2*jc.by*jc.dx))/(jc.cx + jc.dx*(jc.ax*jc.dy + jc.bx*jc.cy + jc.dx*y - jc.ay*jc.dx - jc.by*jc.cx - jc.dy*x - sqrt(-4*jc.bx*jc.cx*jc.dy*y - 4*jc.by*jc.cy*jc.dx*x - 2*jc.ax*jc.ay*jc.dx*jc.dy - 2*jc.ax*jc.bx*jc.cy*jc.dy - 2*jc.ax*jc.by*jc.cx*jc.dy - 2*jc.ay*jc.bx*jc.cy*jc.dx - 2*jc.ay*jc.by*jc.cx*jc.dx - 2*jc.bx*jc.by*jc.cx*jc.cy - 2*jc.dx*jc.dy*x*y + 2*jc.ax*jc.dx*jc.dy*y + 2*jc.ay*jc.dx*jc.dy*x + 2*jc.bx*jc.cy*jc.dx*y + 2*jc.bx*jc.cy*jc.dy*x + 2*jc.by*jc.cx*jc.dx*y + 2*jc.by*jc.cx*jc.dy*x + 4*jc.ax*jc.by*jc.cy*jc.dx + 4*jc.ay*jc.bx*jc.cx*jc.dy + jc.ax*jc.ax*jc.dy*jc.dy + jc.ay*jc.ay*jc.dx*jc.dx + jc.bx*jc.bx*jc.cy*jc.cy + jc.by*jc.by*jc.cx*jc.cx + jc.dx*jc.dx*y*y + jc.dy*jc.dy*x*x - 2*jc.ax*x*jc.dy*jc.dy - 2*jc.ay*y*jc.dx*jc.dx))/(-2*jc.bx*jc.dy + 2*jc.by*jc.dx));
}

/// Compute the gradient with respect to mapped coordinates, i.e. parial derivatives are in terms of the
/// mapped coordinates. The result needs to be multiplied with the inverse jacobian to get the result in real
/// coordinates.
/// @param mappedCoord The mapped coordinates where the gradient should be calculated
/// @param result Storage for the resulting gradient matrix
static void mapped_gradient(const RealVector& mappedCoord, RealMatrix& result)
{
  cf_assert(result.nbRows() == 4);
  cf_assert(result.nbCols() == 2);

  const Real ksi  = mappedCoord[0];
  const Real eta = mappedCoord[1];

  result(0, XX) = 0.25 * (-1 + eta);
  result(0, YY) = 0.25 * (-1 + ksi);
  result(1, XX) = 0.25 * ( 1 - eta);
  result(1, YY) = 0.25 * (-1 - ksi);
  result(2, XX) = 0.25 * ( 1 + eta);
  result(2, YY) = 0.25 * ( 1 + ksi);
  result(3, XX) = 0.25 * (-1 - eta);
  result(3, YY) = 0.25 * ( 1 - ksi);
}

/// Compute the jacobian determinant at the given mapped coordinates
template<typename NodesT>
inline static Real jacobian_determinant(const RealVector& mappedCoord, const NodesT& nodes)
{
  cf_assert(mappedCoord.size() == 2);
  cf_assert(nodes.size() == 4);

  JacobianCoefficients jc(nodes);
  const Real xi  = mappedCoord[0];
  const Real eta = mappedCoord[1];
  return (jc.bx*jc.dy - jc.by*jc.dx)*eta + (jc.dx*jc.cy - jc.cx*jc.dy)*xi + jc.bx*jc.cy - jc.by*jc.cx;
}

/// Compute the Jacobian matrix
/// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
/// @param result Storage for the resulting Jacobian matrix
template<typename NodesT>
static void jacobian(const RealVector& mappedCoord, const NodesT& nodes, RealMatrix& result)
{
  cf_assert(result.nbRows() == 2);
  cf_assert(result.isSquare());

  JacobianCoefficients jc(nodes);

  const Real xi = mappedCoord[KSI];
  const Real eta = mappedCoord[ETA];

  result(KSI,XX) = jc.bx + jc.dx*eta;
  result(KSI,YY) = jc.by + jc.dy*eta;
  result(ETA,XX) = jc.cx + jc.dx*xi;
  result(ETA,YY) = jc.cy + jc.dy*xi;
}

/// Compute the adjoint of Jacobian matrix
/// @param mappedCoord The mapped coordinates where the Jacobian should be calculated
/// @param result Storage for the resulting adjoint
template<typename NodesT>
static void jacobian_adjoint(const RealVector& mappedCoord, const NodesT& nodes, RealMatrix& result) {
  cf_assert(result.nbRows() == 2);
  cf_assert(result.isSquare());

  JacobianCoefficients jc(nodes);

  const Real xi = mappedCoord[KSI];
  const Real eta = mappedCoord[ETA];

  result(KSI,XX) = jc.cy + jc.dy*xi;
  result(KSI,YY) = -jc.by - jc.dy*eta;
  result(ETA,XX) = -jc.cx - jc.dx*xi;
  result(ETA,YY) = jc.bx + jc.dx*eta;
}

/// Volume of the cell
template<typename NodesT>
inline static Real volume(const NodesT& nodes) {
  const Real diagonalsProd =
    (nodes[2][XX] - nodes[0][XX]) * (nodes[3][YY] - nodes[1][YY]) -
    (nodes[2][YY] - nodes[0][YY]) * (nodes[3][XX] - nodes[1][XX]);

  return 0.5*diagonalsProd;
}

/// Number of nodes
static const Uint nb_nodes = 4;

/// Order of the shape function
static const Uint order = 1;

Quad2DLagrangeP1();
virtual std::string getElementTypeName() const;
virtual Real computeVolume(const NodesT& coord) const;

private:
/// Convenience struct to easily access the elements that make up the jacobian
struct JacobianCoefficients
{
  Real ax, bx, cx, dx;
  Real ay, by, cy, dy;
  template<typename NodesT>
  JacobianCoefficients(const NodesT& nodes)
  {
    const Real x0 = nodes[0][XX];
    const Real y0 = nodes[0][YY];
    const Real x1 = nodes[1][XX];
    const Real y1 = nodes[1][YY];
    const Real x2 = nodes[2][XX];
    const Real y2 = nodes[2][YY];
    const Real x3 = nodes[3][XX];
    const Real y3 = nodes[3][YY];

    ax = 0.25*( x0 + x1 + x2 + x3);
    bx = 0.25*(-x0 + x1 + x2 - x3);
    cx = 0.25*(-x0 - x1 + x2 + x3);
    dx = 0.25*( x0 - x1 + x2 - x3);
    ay = 0.25*( y0 + y1 + y2 + y3);
    by = 0.25*(-y0 + y1 + y2 - y3);
    cy = 0.25*(-y0 - y1 + y2 + y3);
    dy = 0.25*( y0 - y1 + y2 - y3);
  }
};

};

} // namespace SF
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_SF_Quad2DLagrangeP1 */
