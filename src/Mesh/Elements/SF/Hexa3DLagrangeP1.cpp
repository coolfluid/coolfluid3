#include "Hexa3DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

Hexa3DLagrangeP1::Hexa3DLagrangeP1()
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

std::string Hexa3DLagrangeP1::getElementTypeName() const
{
  return "P1-Hexa3D";
}

Real Hexa3DLagrangeP1::computeVolume(const NodesT& coord) const
{
  return volume(coord);
}

} // namespace SF
} // namespace Mesh
} // namespace CF
