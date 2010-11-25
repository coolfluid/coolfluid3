// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "Quad3DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Quad3DLagrangeP1,
                         ElementType,
                         LibSF >
aQuad3DLagrangeP1_Builder ( "Quad3DLagrangeP1" );

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
	
bool Quad3DLagrangeP1::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
{
	return false;
}

const ElementType::FaceConnectivity& Quad3DLagrangeP1::face_connectivity() const
{
  static FaceConnectivity connectivity;
  if(connectivity.face_first_nodes.empty())
  {
    connectivity.face_first_nodes = boost::assign::list_of(0);
    connectivity.face_node_counts.assign(1, 4);
    connectivity.face_nodes = boost::assign::list_of(0)(1)(2)(3);
  }
  return connectivity;
}

const CF::Mesh::ElementType& Quad3DLagrangeP1::face_type(const CF::Uint face) const
{
  static const Quad3DLagrangeP1 facetype;
  return facetype;
}




} // SF
} // Mesh
} // CF
