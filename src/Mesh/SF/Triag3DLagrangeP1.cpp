#include "Common/ObjectProvider.hpp"

#include "SFLib.hpp"
#include "Triag3DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Triag3DLagrangeP1,
                         ElementType,
                         SFLib >
aTriag3DLagrangeP1_Provider ( "Triag3DLagrangeP1" );

////////////////////////////////////////////////////////////////////////////////

Triag3DLagrangeP1::Triag3DLagrangeP1() : Triag3D()
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

std::string Triag3DLagrangeP1::getElementTypeName() const
{
  return "Triag3DLagrangeP1";
}

Real Triag3DLagrangeP1::computeVolume(const NodesT& coord) const
{
  return 0;
}

const CF::Mesh::ElementType::FaceConnectivity& Triag3DLagrangeP1::face_connectivity() const
{
  static FaceConnectivity connectivity;
  return connectivity;
}

const CF::Mesh::ElementType& Triag3DLagrangeP1::face_type(const CF::Uint face) const
{
  static const Triag3DLagrangeP1 facetype;
  return facetype;
}

} // namespace SF
} // namespace Mesh
} // namespace CF
