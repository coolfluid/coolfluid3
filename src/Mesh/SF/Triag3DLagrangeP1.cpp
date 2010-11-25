// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "Triag3DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Triag3DLagrangeP1,
                         ElementType,
                         LibSF >
aTriag3DLagrangeP1_Builder ( "Triag3DLagrangeP1" );

////////////////////////////////////////////////////////////////////////////////

Triag3DLagrangeP1::Triag3DLagrangeP1(const std::string& name) : Triag3D(name)
{
  add_tag( type_name() );

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

bool Triag3DLagrangeP1::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
{
	return false;
}

const CF::Mesh::ElementType::FaceConnectivity& Triag3DLagrangeP1::face_connectivity() const
{
  static FaceConnectivity connectivity;
  if(connectivity.face_first_nodes.empty())
  {
    connectivity.face_first_nodes = boost::assign::list_of(0);
    connectivity.face_node_counts.assign(1, 3);
    connectivity.face_nodes = boost::assign::list_of(0)(1)(2);
  }
  return connectivity;
}

const CF::Mesh::ElementType& Triag3DLagrangeP1::face_type(const CF::Uint face) const
{
  static const Triag3DLagrangeP1 facetype;
  return facetype;
}

} // SF
} // Mesh
} // CF
