// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "Hexa3DLagrangeP1.hpp"
#include "Quad3DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Hexa3DLagrangeP1,
                           ElementType,
                           LibSF >
aHexa3DLagrangeP1_Builder ( "Hexa3DLagrangeP1" );

////////////////////////////////////////////////////////////////////////////////

Hexa3DLagrangeP1::Hexa3DLagrangeP1(const std::string& name) : Hexa3D(name)
{
  add_tag( type_name() );

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
	
bool Hexa3DLagrangeP1::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
{
	return in_element(coord,nodes);
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

	
bool Hexa3DLagrangeP1::in_element( const CoordsT& coord, const NodeMatrixT& nodes)
{
  
  for (Uint iFace=0; iFace<nb_faces; ++iFace)
  {
    if (!(is_orientation_inside(coord, nodes, iFace)))
      return false;
  }
  return true;
}

bool Hexa3DLagrangeP1::is_orientation_inside(const CoordsT& coord, const NodeMatrixT& nodes, const Uint face)
{
  //test according to http://graphics.ethz.ch/~peikert/personal/HexCellTest/
  
  const Uint a = faces().face_node_range(face)[3];
  const Uint b = faces().face_node_range(face)[2];
  const Uint c = faces().face_node_range(face)[1];
  const Uint d = faces().face_node_range(face)[0];
  
  RealMatrix3 M;
  M.col(0) = nodes.row(b) - nodes.row(a);
  M.col(1) = nodes.row(d) - nodes.row(a);
  M.col(2) = nodes.row(c) - nodes.row(a);
  const CoordsT pp = coord.transpose()  - nodes.row(a);
  
  const CoordsT bp_x_dp = M.col(0).cross(M.col(1));
  const Real h = bp_x_dp.dot(M.col(2));
  if (h != 0)
  {
    RealMatrix3 T;
    T << 1,  0,  1,
         0,  1,  1,
         0,  0,  h;
    
    // Do transformation
    RealVector ppp = T*M.inverse()*pp;
    
    if (ppp[ZZ] < h*ppp[XX]*ppp[YY])
      return false;
  }
  else
  {
    if (bp_x_dp.dot(pp) < 0)
      return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
