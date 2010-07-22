#include "Common/ObjectProvider.hpp"

#include "SFLib.hpp"
#include "Quad3DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Quad3DLagrangeP1,
                         ElementType,
                         SFLib >
aQuad3DLagrangeP1_Provider ( "Quad3DLagrangeP1" );

////////////////////////////////////////////////////////////////////////////////

Quad3DLagrangeP1::Quad3DLagrangeP1() : Quad3D()
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

std::string Quad3DLagrangeP1::getElementTypeName() const
{
  return "Quad3DLagrangeP1";
}

Real Quad3DLagrangeP1::computeVolume(const NodesT& coord) const
{
  return 0;
}

const ElementType::FaceConnectivity& Quad3DLagrangeP1::face_connectivity() const
{
  static FaceConnectivity connectivity;
  return connectivity;
}

const CF::Mesh::ElementType& Quad3DLagrangeP1::face_type(const CF::Uint face) const
{
  static const Quad3DLagrangeP1 facetype;
  return facetype;
}




} // namespace SF
} // namespace Mesh
} // namespace CF
