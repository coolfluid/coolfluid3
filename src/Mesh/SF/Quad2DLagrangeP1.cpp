// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ObjectProvider.hpp"

#include "LibSF.hpp"
#include "Line2DLagrangeP1.hpp"
#include "Quad2DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Quad2DLagrangeP1,
                         ElementType,
                         LibSF >
aQuad2DLagrangeP1_Provider ( "Quad2DLagrangeP1" );

////////////////////////////////////////////////////////////////////////////////

Quad2DLagrangeP1::Quad2DLagrangeP1()
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

std::string Quad2DLagrangeP1::getElementTypeName() const
{
  return "Quad2DLagrangeP1";
}

Real Quad2DLagrangeP1::computeVolume(const NodesT& coord) const
{
  return volume(coord);
}
	
bool Quad2DLagrangeP1::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
{
	MappedCoordsT mapped_coord;
	mapped_coordinates(CoordsT(coord), nodes, mapped_coord);
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

const CF::Mesh::ElementType::FaceConnectivity& Quad2DLagrangeP1::faces()
{
  static FaceConnectivity connectivity;
  if(connectivity.face_first_nodes.empty())
  {
    connectivity.face_first_nodes = boost::assign::list_of(0)(2)(4)(6);
    connectivity.face_node_counts.assign(4, 2);
    connectivity.face_nodes = boost::assign::list_of(0)(1)
                                                    (1)(2)
                                                    (2)(3)
                                                    (3)(0);
  }
  return connectivity;
}

const CF::Mesh::ElementType::FaceConnectivity& Quad2DLagrangeP1::face_connectivity() const
{
  return faces();
}

const CF::Mesh::ElementType& Quad2DLagrangeP1::face_type(const CF::Uint face) const
{
  const static Line2DLagrangeP1 facetype;
  return facetype;
}


} // namespace SF
} // namespace Mesh
} // namespace CF
