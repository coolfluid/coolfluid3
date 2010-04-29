#ifndef CF_Mesh_LagrangeSF_TriagP1_hpp
#define CF_Mesh_LagrangeSF_TriagP1_hpp

#include "Common/AssertionManager.hpp"
#include "Mesh/LagrangeSF/LagrangeSF.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeSF {

/// This class provides the lagrangian shape function describing the
/// representation of the solution and/or the geometry in a P1 (linear)
/// triangular element.
/// @author Andrea Lani
/// @author Geoffrey Deliege
/// @author Tiago Quintino
/// @author Bart Janssens
class TriagP1 {
public:

/// Compute the shape functions corresponding to the given
/// mapped coordinates
/// @param mappedCoord The mapped coordinates
/// @param shapeFunc Vector storing the result
static void computeShapeFunction(const RealVector& mappedCoord, RealVector& shapeFunc) {
  cf_assert(shapeFunc.size() == 3);
  cf_assert(mappedCoord.size() == 2);
  shapeFunc[0] = 1.0 - mappedCoord[0] - mappedCoord[1];
  shapeFunc[1] = mappedCoord[0];
  shapeFunc[2] = mappedCoord[1];
}

/// Compute the jacobian determinant at the given
/// mapped coordinates
inline static Real computeJacobianDeterminant(const RealVector& mappedCoord, const NodesT& nodes) {
  return   (nodes[1][XX] - nodes[0][XX]) * (nodes[2][YY] - nodes[0][YY])
         - (nodes[2][XX] - nodes[0][XX]) * (nodes[1][YY] - nodes[0][YY]);
}

/// Compute Mapped Coordinates
/// @param coord contains the coordinates to be mapped
/// @param nodes contains the nodes
/// @param mappedCoord Store the output mapped coordinates
static void computeMappedCoordinates(const RealVector& coord, const NodesT& nodes, RealVector& mappedCoord) {
  cf_assert(coord.size() == 2);
  cf_assert(mappedCoord.size() == 2);
  cf_assert(nodes.size() == 3);
  const Real invDet = 1. / computeJacobianDeterminant(mappedCoord, nodes);
  mappedCoord[KSI] = invDet * ((nodes[2][YY] - nodes[0][YY])*coord[XX] + (nodes[0][XX] - nodes[2][XX])*coord[YY] - nodes[0][XX]*nodes[2][YY] + nodes[2][XX]*nodes[0][YY]);
  mappedCoord[ETA] = invDet * ((nodes[0][YY] - nodes[1][YY])*coord[XX] + (nodes[1][XX] - nodes[0][XX])*coord[YY] + nodes[0][XX]*nodes[1][YY] - nodes[1][XX]*nodes[0][YY]);
}



private:
/// Cannot be instantiated
TriagP1() {}

/// Cannot be destroyed
~TriagP1() {}

};

} // namespace LagrangeSF
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_LagrangeSF_TriagP1 */
