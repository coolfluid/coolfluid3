#include "Triag3DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

Triag3DLagrangeP1::Triag3DLagrangeP1() : Triag3D()
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

std::string Triag3DLagrangeP1::getElementTypeName() const
{
  return "P1-Triag3D";
}

Real Triag3DLagrangeP1::computeVolume(const NodesT& coord) const
{
  return 0;
}

} // namespace SF
} // namespace Mesh
} // namespace CF
