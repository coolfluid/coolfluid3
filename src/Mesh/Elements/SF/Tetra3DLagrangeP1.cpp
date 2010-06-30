#include "Tetra3DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

Tetra3DLagrangeP1::Tetra3DLagrangeP1()
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

std::string Tetra3DLagrangeP1::getElementTypeName() const
{
  return "P1-Tetra3D";
}

Real Tetra3DLagrangeP1::computeVolume(const NodesT& coord) const
{
  return volume(coord);
}

} // namespace SF
} // namespace Mesh
} // namespace CF
