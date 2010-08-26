#include "Common/ObjectProvider.hpp"

#include "SFLib.hpp"
#include "Line2DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Line2DLagrangeP1,
                         ElementType,
                         SFLib >
aLine2DLagrangeP1_Provider ( "Line2DLagrangeP1" );

////////////////////////////////////////////////////////////////////////////////

Line2DLagrangeP1::Line2DLagrangeP1() : Line2D()
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

std::string Line2DLagrangeP1::getElementTypeName() const
{
  return "Line2DLagrangeP1";
}

Real Line2DLagrangeP1::computeVolume(const NodesT& coord) const
{
  return 0;
}

const CF::Mesh::ElementType::FaceConnectivity& Line2DLagrangeP1::face_connectivity() const
{
  static FaceConnectivity connectivity;
  if(connectivity.face_first_nodes.empty())
  {
    connectivity.face_first_nodes = boost::assign::list_of(0);
    connectivity.face_node_counts.assign(1, 2);
    connectivity.face_nodes = boost::assign::list_of(0)(1);
  }
  return connectivity;
}

const CF::Mesh::ElementType& Line2DLagrangeP1::face_type(const CF::Uint face) const
{
  static const Line2DLagrangeP1 facetype;
  return facetype;
}



} // namespace SF
} // namespace Mesh
} // namespace CF
