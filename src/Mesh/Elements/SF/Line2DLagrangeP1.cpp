#include "Line2DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

Line2DLagrangeP1::Line2DLagrangeP1()
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

std::string Line2DLagrangeP1::getElementTypeName() const
{
  return "P1-Line2D";
}

Real Line2DLagrangeP1::computeVolume(const NodesT& coord) const
{
  return 0;
}

} // namespace SF
} // namespace Mesh
} // namespace CF
