#include "Common/ObjectProvider.hpp"

#include "SFLib.hpp"
#include "Line1DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Line1DLagrangeP1,
                         ElementType,
                         SFLib >
aLine1DLagrangeP1_Provider ( "Line1DLagrangeP1" );

////////////////////////////////////////////////////////////////////////////////
Line1DLagrangeP1::Line1DLagrangeP1()
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

std::string Line1DLagrangeP1::getElementTypeName() const
{
  return "Line1DLagrangeP1";
}

Real Line1DLagrangeP1::computeVolume(const NodesT& coord) const
{
  return volume(coord);
}

const CF::Mesh::ElementType::FaceConnectivity& Line1DLagrangeP1::face_connectivity() const
{
  static FaceConnectivity connectivity;
  return connectivity;
}

const CF::Mesh::ElementType& Line1DLagrangeP1::face_type(const CF::Uint face) const
{
  // TODO: Add a Point1DLagrangeP1 type to complete this
  throw NotImplemented(FromHere(), "Line1DLagrangeP1::face_type requires a point type");
}


} // namespace SF
} // namespace Mesh
} // namespace CF
