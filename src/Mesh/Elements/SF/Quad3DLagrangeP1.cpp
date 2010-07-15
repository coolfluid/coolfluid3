#include "Quad3DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

Quad3DLagrangeP1::Quad3DLagrangeP1() : Quad3D()
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

std::string Quad3DLagrangeP1::getElementTypeName() const
{
  return "P1-Quad3D";
}

Real Quad3DLagrangeP1::computeVolume(const NodesT& coord) const
{
  return 0;
}

} // namespace SF
} // namespace Mesh
} // namespace CF
