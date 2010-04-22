#ifndef CF_Mesh_LagrangeSF_P1_HH
#define CF_Mesh_LagrangeSF_P1_HH

#include "Common/AssertionManager.hpp"
#include "Math/RealVector.hpp"

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
  shapeFunc[0] = 1.0 - mappedCoord[0] - mappedCoord[1];
  shapeFunc[1] = mappedCoord[0];
  shapeFunc[2] = mappedCoord[1];
}

/// Compute Mapped Coordinates
/// @param coord contains the coordinates to be mapped
/// @param nodes contains the nodes
/// @return RealVector containing the Mapped Coordinates
//static RealVector computeMappedCoordinates(const RealVector& coord, const std::vector<Framework::Node*>& nodes) {
//
//}

private:
/// Cannot be instantiated
TriagP1() {}

/// Cannot be destroyed
~TriagP1() {}

};

} // namespace LagrangeSF
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_LagrangeSF_P1 */
