#include "Line3DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

Line3DLagrangeP1::Line3DLagrangeP1() : Line3D()
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

std::string Line3DLagrangeP1::getElementTypeName() const
{
  return "P1-Line3D";
}

Real Line3DLagrangeP1::computeVolume(const NodesT& coord) const
{
  return 0;
}

} // namespace SF
} // namespace Mesh
} // namespace CF
