#include "Quad2DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

Quad2DLagrangeP1::Quad2DLagrangeP1()
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

std::string Quad2DLagrangeP1::getElementTypeName() const
{
  return "P1-Quad2D";
}

Real Quad2DLagrangeP1::computeVolume(const NodesT& coord) const
{
  return volume(coord);
}

} // namespace SF
} // namespace Mesh
} // namespace CF
