// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ObjectProvider.hpp"

#include "LibSF.hpp"
#include "Line2DLagrangeP1.hpp"
#include "Quad2DLagrangeP2.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Quad2DLagrangeP2,
                         ElementType,
                         LibSF >
aQuad2DLagrangeP2_Provider ( "Quad2DLagrangeP2" );

////////////////////////////////////////////////////////////////////////////////

Quad2DLagrangeP2::Quad2DLagrangeP2()
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

std::string Quad2DLagrangeP2::getElementTypeName() const
{
  return "Quad2DLagrangeP2";
}

Real Quad2DLagrangeP2::computeVolume(const NodesT& coord) const
{
  return volume(coord);
}
	
bool Quad2DLagrangeP2::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
{
  // @todo: this was copied & pasted from P1Quad code
  // make sure it works in P2 case as well
	RealVector mapped_coord(coord.size());
	mapped_coordinates(coord, nodes, mapped_coord);
	if( (mapped_coord[KSI] >= -0.5) &&
			(mapped_coord[ETA] >= -0.5) &&
		  (mapped_coord[KSI] <=  0.5) &&
		  (mapped_coord[ETA] <=  0.5))
	{
		return true;
	}
	else
	{
		return false;
	}
}

const CF::Mesh::ElementType::FaceConnectivity& Quad2DLagrangeP2::faces()
{
  static FaceConnectivity connectivity;
  if(connectivity.face_first_nodes.empty())
  {
    connectivity.face_first_nodes = boost::assign::list_of(0)(3)(6)(9);
    connectivity.face_node_counts.assign(4, 3);
    connectivity.face_nodes = boost::assign::list_of(0)(1)(4)
                                                    (1)(2)(5)
                                                    (2)(3)(6)
                                                    (3)(0)(7);
  }
  return connectivity;
}

const CF::Mesh::ElementType::FaceConnectivity& Quad2DLagrangeP2::face_connectivity() const
{
  return faces();
}

const CF::Mesh::ElementType& Quad2DLagrangeP2::face_type(const CF::Uint face) const
{
  const static Line2DLagrangeP1 facetype;
  return facetype;
}

Real Quad2DLagrangeP2::jacobian_determinantV ( const CF::RealVector& mapped_coord, const CF::Mesh::ElementType::NodesT& nodes ) const
{
  return jacobian_determinant(mapped_coord, nodes);
}



} // namespace SF
} // namespace Mesh
} // namespace CF
