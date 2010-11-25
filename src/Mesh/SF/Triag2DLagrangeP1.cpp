// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "Triag2DLagrangeP1.hpp"
#include "Line2DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Triag2DLagrangeP1,
                         ElementType,
                         LibSF >
aTriag2DLagrangeP1_Builder ( "Triag2DLagrangeP1" );

////////////////////////////////////////////////////////////////////////////////

Triag2DLagrangeP1::Triag2DLagrangeP1(const std::string& name) : Triag2D(name)
{
  add_tag( type_name() );

  m_nb_nodes = nb_nodes;
  m_order = order;
}

std::string Triag2DLagrangeP1::getElementTypeName() const
{
  return "Triag2DLagrangeP1";
}

Real Triag2DLagrangeP1::computeVolume(const NodesT& coord) const
{
  return volume(coord);
}
	
bool Triag2DLagrangeP1::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
{
	return in_element(coord,nodes);
}

const CF::Mesh::ElementType::FaceConnectivity& Triag2DLagrangeP1::faces()
{
  static FaceConnectivity connectivity;
  if(connectivity.face_first_nodes.empty())
  {
    connectivity.face_first_nodes = boost::assign::list_of(0)(2)(4);
    connectivity.face_node_counts.assign(nb_nodes, 2);
    connectivity.face_nodes = boost::assign::list_of(0)(1)
                                                    (1)(2)
                                                    (2)(0);
  }
  return connectivity;
}

const CF::Mesh::ElementType::FaceConnectivity& Triag2DLagrangeP1::face_connectivity() const
{
  return faces();
}

const CF::Mesh::ElementType& Triag2DLagrangeP1::face_type(const CF::Uint face) const
{
  static const Line2DLagrangeP1 facetype;
  return facetype;
}




} // SF
} // Mesh
} // CF
