// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ObjectProvider.hpp"

#include "LibSF.hpp"
#include "Point1DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Point1DLagrangeP1,
                         ElementType,
                         LibSF >
aPoint1DLagrangeP1_Provider ( "Point1DLagrangeP1" );

////////////////////////////////////////////////////////////////////////////////

Point1DLagrangeP1::Point1DLagrangeP1() : Point1D()
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

std::string Point1DLagrangeP1::getElementTypeName() const
{
  return "Point1DLagrangeP1";
}

Real Point1DLagrangeP1::computeVolume(const NodesT& coord) const
{
  return 0;
}

bool Point1DLagrangeP1::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
{
  return false;
}

const CF::Mesh::ElementType::FaceConnectivity& Point1DLagrangeP1::face_connectivity() const
{
  static FaceConnectivity connectivity;
  if(connectivity.face_first_nodes.empty())
  {
    connectivity.face_first_nodes = boost::assign::list_of(0);
    connectivity.face_node_counts.assign(1, 1);
    connectivity.face_nodes = boost::assign::list_of(0);
  }
  return connectivity;
}

const CF::Mesh::ElementType& Point1DLagrangeP1::face_type(const CF::Uint face) const
{
  static const Point1DLagrangeP1 facetype;
  return facetype;
}

} // SF
} // Mesh
} // CF
