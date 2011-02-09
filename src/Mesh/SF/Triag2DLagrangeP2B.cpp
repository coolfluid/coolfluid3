// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "Triag2DLagrangeP2B.hpp"
#include "Line2DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Triag2DLagrangeP2B,
                         ElementType,
                         LibSF >
aTriag2DLagrangeP2B_Builder;

////////////////////////////////////////////////////////////////////////////////

Triag2DLagrangeP2B::Triag2DLagrangeP2B(const std::string& name) : Triag2D(name)
{
   

  m_nb_nodes = nb_nodes;
  m_order = order;
}

std::string Triag2DLagrangeP2B::element_type_name() const
{
  return type_name();
}

Real Triag2DLagrangeP2B::compute_volume(const NodesT& coord) const
{
  return volume(coord);
}
	
bool Triag2DLagrangeP2B::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
{
	return in_element(coord,nodes);
}

const CF::Mesh::ElementType::FaceConnectivity& Triag2DLagrangeP2B::faces()
{
  throw Common::NotImplemented( FromHere(), "" );

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

const CF::Mesh::ElementType::FaceConnectivity& Triag2DLagrangeP2B::face_connectivity() const
{
  return faces();
}

const CF::Mesh::ElementType& Triag2DLagrangeP2B::face_type(const CF::Uint face) const
{
  throw Common::NotImplemented( FromHere(), "" );

  static const Line2DLagrangeP1 facetype;
  return facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Triag2DLagrangeP2B::shape_function(const MappedCoordsT& map_coord, ShapeFunctionsT& shapef)
{
  const Real L0 = 1.0 - map_coord[0] - map_coord[1];
  const Real L1 = map_coord[0];
  const Real L2 = map_coord[1];

  const Real Phi6 = L0 * L1 * L2;

  shapef[0] = ( 2*L0 - 1.0 ) * L0 + 3 * Phi6 ;
  shapef[1] = ( 2*L1 - 1.0 ) * L1 + 3 * Phi6 ;
  shapef[2] = ( 2*L2 - 1.0 ) * L2 + 3 * Phi6 ;
  shapef[3] = 4*L0*L1 - 12. * Phi6 ;
  shapef[4] = 4*L1*L2 - 12. * Phi6 ;
  shapef[5] = 4*L2*L0 - 12. * Phi6 ;
  shapef[6] = 27 * Phi6;

}

////////////////////////////////////////////////////////////////////////////////

void Triag2DLagrangeP2B::mapped_coordinates(const CoordsT& coord, const NodeMatrixT& nodes, MappedCoordsT& map_coord)
{
  throw Common::NotImplemented( FromHere(), "" );
  
  const Real invDet = 1. / jacobian_determinant(nodes);

  map_coord[KSI] = invDet * ((nodes(2, YY) - nodes(0, YY))*coord[XX] + (nodes(0, XX) - nodes(2, XX))*coord[YY] - nodes(0, XX)*nodes(2, YY) + nodes(2, XX)*nodes(0, YY));
  map_coord[ETA] = invDet * ((nodes(0, YY) - nodes(1, YY))*coord[XX] + (nodes(1, XX) - nodes(0, XX))*coord[YY] + nodes(0, XX)*nodes(1, YY) - nodes(1, XX)*nodes(0, YY));

}

////////////////////////////////////////////////////////////////////////////////

void Triag2DLagrangeP2B::mapped_gradient(const MappedCoordsT& map_coord, MappedGradientT& result)
{
  const Real L0 = 1.0 - map_coord[0] - map_coord[1];
  const Real L1 = map_coord[0];
  const Real L2 = map_coord[1];

  const Real L0L1 = L0*L1;
  const Real L1L2 = L1*L2;
  const Real L2L0 = L2*L0;

  const Real L2L0_L1L2 = L2L0 - L1L2;
  const Real L0L1_L2L1 = L0L1 - L1L2;

  result(XX, 0) = - (4*L0-1)  +  3*(L2L0_L1L2);
  result(YY, 0) = - (4*L0-1)  +  3*(L0L1_L2L1);

  result(XX, 1) =   (4*L1-1)  +  3*(L2L0_L1L2);
  result(YY, 1) =                3*(L0L1_L2L1);

  result(XX, 2) =                3*(L2L0_L1L2);
  result(YY, 2) =   (4*L2-1)  +  3*(L0L1_L2L1);

  result(XX, 3) =   4*(L0-L1) - 12*(L2L0_L1L2);
  result(YY, 3) = - 4*L1      - 12*(L0L1_L2L1);

  result(XX, 4) =   4*L2      - 12*(L2L0_L1L2);
  result(YY, 4) =   4*L1      - 12*(L0L1_L2L1);

  result(XX, 5) = - 4*L2      - 12*(L2L0_L1L2);
  result(YY, 5) =   4*(L0-L2) - 12*(L0L1_L2L1);

  result(XX, 6) =   27*L2L0_L1L2 ;
  result(YY, 6) =   27*L0L1_L2L1 ;

}

////////////////////////////////////////////////////////////////////////////////

Real Triag2DLagrangeP2B::jacobian_determinant(const MappedCoordsT& map_coord, const NodeMatrixT& nodes)
{
  return jacobian_determinant(nodes);
}

////////////////////////////////////////////////////////////////////////////////

void Triag2DLagrangeP2B::jacobian(const MappedCoordsT& map_coord, const NodeMatrixT& nodes, JacobianT& result)
{

    /// @todo function Triag2DLagrangeP2B::jacobian needs to be updated for curvilinear elements

  result(KSI,XX) = nodes(1, XX) - nodes(0, XX);
  result(KSI,YY) = nodes(1, YY) - nodes(0, YY);
  result(ETA,XX) = nodes(2, XX) - nodes(0, XX);
  result(ETA,YY) = nodes(2, YY) - nodes(0, YY);
}

////////////////////////////////////////////////////////////////////////////////

void Triag2DLagrangeP2B::jacobian_adjoint(const MappedCoordsT& map_coord, const NodeMatrixT& nodes, JacobianT& result)
{
  throw Common::NotImplemented( FromHere(), "" );

}

////////////////////////////////////////////////////////////////////////////////

Real Triag2DLagrangeP2B::volume(const NodeMatrixT& nodes)
{
  throw Common::NotImplemented( FromHere(), "" );
}

////////////////////////////////////////////////////////////////////////////////

bool Triag2DLagrangeP2B::in_element(const CoordsT& coord, const NodeMatrixT& nodes)
{
  throw Common::NotImplemented( FromHere(), "" );

  MappedCoordsT mapped_coord;
  mapped_coordinates(coord, nodes, mapped_coord);
  if( (mapped_coord[KSI] >= -Math::MathConsts::eps()) &&
      (mapped_coord[ETA] >= -Math::MathConsts::eps()) &&
      (mapped_coord.sum() <= 1.))
  {
    return true;
  }
  else
  {
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

Real Triag2DLagrangeP2B::jacobian_determinant(const NodeMatrixT& nodes)
{
  throw Common::NotImplemented( FromHere(), "" );

  const Real x0 = nodes(0, XX);
  const Real x1 = nodes(1, XX);
  const Real x2 = nodes(2, XX);
  const Real y0 = nodes(0, YY);
  const Real y1 = nodes(1, YY);
  const Real y2 = nodes(2, YY);

  return (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0);
}

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
