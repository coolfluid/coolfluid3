#include "Triag2DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

Triag2DLagrangeP1::Triag2DLagrangeP1()
{
    m_nb_nodes = nb_nodes;
    m_order = order;
}

std::string Triag2DLagrangeP1::getElementTypeName() const
{
  return "P1-Triag2D";
}

Real Triag2DLagrangeP1::computeVolume(const NodesT& coord) const
{
  return volume(coord);
}

} // namespace SF
} // namespace Mesh
} // namespace CF
