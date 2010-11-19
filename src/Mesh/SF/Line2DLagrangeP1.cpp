// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ObjectProvider.hpp"

#include "LibSF.hpp"
#include "Line2DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Line2DLagrangeP1,
                         ElementType,
                         LibSF >
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

bool Line2DLagrangeP1::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
{
	return false;
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



} // SF
} // Mesh
} // CF
