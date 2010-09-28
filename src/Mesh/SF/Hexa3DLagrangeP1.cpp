// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/ObjectProvider.hpp"

#include "LibSF.hpp"
#include "Hexa3DLagrangeP1.hpp"
#include "Quad3DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < Hexa3DLagrangeP1,
                         ElementType,
                         LibSF >
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

	
bool Hexa3DLagrangeP1::in_element(const RealVector& coord, const ElementType::NodesT& nodes)
{
	//  This would be easier if mapped_coord function were implemented
	//	RealVector mapped_coord(coord.size());
	//	mapped_coordinates(coord, nodes, mapped_coord);
	//	if((mapped_coord[KSI] >= -1.0) &&
	//		 (mapped_coord[ETA] >= -1.0) &&
	//		 (mapped_coord[ZTA] >= -1.0) &&
	//		 (mapped_coord[KSI] <=  1.0) &&
	//		 (mapped_coord[ETA] <=  1.0) &&
	//		 (mapped_coord[ZTA] <=  1.0))
	//	{
	//		return true;
	//	}
	//	else
	//	{
	//		return false;
	//	}
	
	for (Uint iFace=0; iFace<nb_faces; ++iFace)
	{
		if (!(is_orientation_inside(coord, nodes, iFace)))
			return false;
	}
	return true;
}

bool Hexa3DLagrangeP1::is_orientation_inside(const RealVector& coord, const NodesT& nodes, const Uint face)
{
	//test according to http://graphics.ethz.ch/~peikert/personal/HexCellTest/
	
	RealVector p(coord);
	RealVector a(nodes[faces().face_node_range(face)[3]]);
	RealVector b(nodes[faces().face_node_range(face)[2]]);
	RealVector c(nodes[faces().face_node_range(face)[1]]);
	RealVector d(nodes[faces().face_node_range(face)[0]]);
	
	RealVector bp = b - a;
	RealVector cp = c - a;
	RealVector dp = d - a;
	RealVector pp = p - a;
	
	
	RealVector bp_x_dp(3);
	Math::MathFunctions::crossProd(bp,dp,bp_x_dp);
	Real h = Math::MathFunctions::innerProd(bp_x_dp,cp);
	if (h != 0)
	{
		RealMatrix T(3,3);
		T(0,0) = 1;		T(0,1) = 0;		T(0,2) = 1;
		T(1,0) = 0;		T(1,1) = 1;		T(1,2) = 1;
		T(2,0) = 0;		T(2,1) = 0;		T(2,2) = h;
		
		RealMatrix M(3,3);
		M(0,0) = bp[XX];		M(0,1) = dp[XX];		M(0,2) = cp[XX];
		M(1,0) = bp[YY];		M(1,1) = dp[YY];		M(1,2) = cp[YY];
		M(2,0) = bp[ZZ];		M(2,1) = dp[ZZ];		M(2,2) = cp[ZZ];
		
		RealMatrix M_inv(3,3);
		Math::MatrixInverterT<3> inverter;
		inverter.invert(M,M_inv);
		RealMatrix transformation(3,3);
		transformation = T*M_inv;
		
		// Do transformation
		RealVector ppp = transformation*pp;
		
		if (ppp[ZZ] < h*ppp[XX]*ppp[YY])
			return false;
	}
	else
	{
		if (Math::MathFunctions::innerProd(bp_x_dp,pp) < 0)
			return false;
	}
	return true;
}

Real Hexa3DLagrangeP1::jacobian_determinantV ( const CF::RealVector& mapped_coord, const CF::Mesh::ElementType::NodesT& nodes ) const
{
  return jacobian_determinant(mapped_coord, nodes);
}


////////////////////////////////////////////////////////////////////////////////

} // namespace SF
} // namespace Mesh
} // namespace CF
