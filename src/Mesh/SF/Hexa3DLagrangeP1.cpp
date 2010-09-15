// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/ObjectProvider.hpp"

#include "SFLib.hpp"
#include "Hexa3DLagrangeP1.hpp"
#include "Quad3DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Hexa3DLagrangeP1,
                         ElementType,
                         SFLib >
aHexa3DLagrangeP1_Provider ( "Hexa3DLagrangeP1" );

////////////////////////////////////////////////////////////////////////////////

Hexa3DLagrangeP1::Hexa3DLagrangeP1()
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

std::string Hexa3DLagrangeP1::getElementTypeName() const
{
  return "Hexa3DLagrangeP1";
}

Real Hexa3DLagrangeP1::computeVolume(const NodesT& coord) const
{
  return volume(coord);
}

const ElementType::FaceConnectivity& Hexa3DLagrangeP1::faces()
{
  static FaceConnectivity connectivity;
  if(connectivity.face_first_nodes.empty())
  {
    connectivity.face_first_nodes = boost::assign::list_of(0)(4)(8)(12)(16)(20);
    connectivity.face_node_counts.assign(6, 4);
    connectivity.face_nodes = boost::assign::list_of(0)(3)(2)(1)
                                                    (4)(5)(6)(7)
                                                    (0)(1)(5)(4)
                                                    (1)(2)(6)(5)
                                                    (3)(7)(6)(2)
                                                    (0)(4)(7)(3);
  }
  return connectivity;
}

const CF::Mesh::ElementType::FaceConnectivity& Hexa3DLagrangeP1::face_connectivity() const
{
  return faces();
}

const CF::Mesh::ElementType& Hexa3DLagrangeP1::face_type(const CF::Uint face) const
{
  static const Quad3DLagrangeP1 facetype;
  return facetype;
}


////////////////////////////////////////////////////////////////////////////////

} // namespace SF
} // namespace Mesh
} // namespace CF
