#include "Line1DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

Line1DLagrangeP1::Line1DLagrangeP1()
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

std::string Line1DLagrangeP1::getElementTypeName() const
{
  return "P1-Line1D";
}

Real Line1DLagrangeP1::computeVolume(const NodesT& coord) const
{
  return volume(coord);
}

} // namespace SF
} // namespace Mesh
} // namespace CF
