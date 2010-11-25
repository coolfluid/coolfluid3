// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Mesh/SF/LibSF.hpp"
#include "Mesh/SF/Tetra3DLagrangeP1.hpp"
#include "Mesh/SF/Triag3DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Tetra3DLagrangeP1,
                         ElementType,
                         LibSF >
aTetra3DLagrangeP1_Builder ( "Tetra3DLagrangeP1" );

////////////////////////////////////////////////////////////////////////////////

Tetra3DLagrangeP1::Tetra3DLagrangeP1(const std::string& name) : Hexa3D(name)
{
  add_tag( type_name() );

  m_nb_nodes = nb_nodes;
  m_order = order;
}

std::string Tetra3DLagrangeP1::getElementTypeName() const
{
  return "Tetra3DLagrangeP1";
}

Real Tetra3DLagrangeP1::computeVolume(const NodesT& coord) const
{
  return volume(coord);
}
	
bool Tetra3DLagrangeP1::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
{
	return in_element(coord,nodes);
}

const CF::Mesh::ElementType::FaceConnectivity& Tetra3DLagrangeP1::faces()
{
  static FaceConnectivity connectivity;
  if(connectivity.face_first_nodes.empty())
  {
    connectivity.face_first_nodes = boost::assign::list_of(0)(3)(6)(9);
    connectivity.face_node_counts.assign(4, 3);
    connectivity.face_nodes = boost::assign::list_of(0)(2)(1)
                                                    (0)(1)(3)
                                                    (1)(2)(3)
                                                    (0)(3)(2);
  }
  return connectivity;
}

const CF::Mesh::ElementType::FaceConnectivity& Tetra3DLagrangeP1::face_connectivity() const
{
  return faces();
}

const CF::Mesh::ElementType& Tetra3DLagrangeP1::face_type(const CF::Uint face) const
{
  const static Triag3DLagrangeP1 facetype;
  return facetype;
}




} // SF
} // Mesh
} // CF
