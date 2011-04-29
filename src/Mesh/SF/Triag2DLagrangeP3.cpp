// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/BasicExceptions.hpp"
#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "Triag2DLagrangeP3.hpp"
//#include "Line2DLagrangeP2.hpp"  //@todo: create Line2DLagrangeP2.hpp

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Triag2DLagrangeP3,ElementType,LibSF > aTriag2DLagrangeP3_Builder;

////////////////////////////////////////////////////////////////////////////////

Triag2DLagrangeP3::Triag2DLagrangeP3(const std::string& name) : Triag2D(name)
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

////////////////////////////////////////////////////////////////////////////////

Real Triag2DLagrangeP3::compute_volume(const NodesT& coord) const
{
  return volume(coord);
}

////////////////////////////////////////////////////////////////////////////////

bool Triag2DLagrangeP3::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
{
	return in_element(coord,nodes);
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Triag2DLagrangeP3::faces()
{
  throw Common::NotImplemented( FromHere(), "" );

  static FaceConnectivity connectivity;

  if(connectivity.face_first_nodes.empty())
  {
    connectivity.face_first_nodes = boost::assign::list_of(0)(4)(8);
    connectivity.face_node_counts.assign(nb_nodes, 4);
    connectivity.face_nodes = boost::assign::list_of(0)(1)(3)(4)
                                                    (1)(2)(5)(6)
                                                    (2)(0)(7)(8);
  }
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Triag2DLagrangeP3::face_connectivity() const
{
  return faces();
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType& Triag2DLagrangeP3::face_type(const CF::Uint face) const
{
  throw Common::NotImplemented( FromHere(), "" );

  //static const Line2DLagrangeP3 facetype;
  //return facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Triag2DLagrangeP3::shape_function_value(const MappedCoordsT& map_coord, ShapeFunctionsT& shapef)
{
  const Real L0 = 1.0 - map_coord[0] - map_coord[1];
  const Real L1 = map_coord[0];
  const Real L2 = map_coord[1];

  shapef[0] = 0.5*( 3*L0 - 1. )*( 3*L0 - 2.)*L0;
  shapef[1] = 0.5*( 3*L1 - 1. )*( 3*L1 - 2.)*L1;
  shapef[2] = 0.5*( 3*L2 - 1. )*( 3*L2 - 2.)*L2;

  shapef[3] = 9./2. * L0*L1*( 3*L0 - 1. );
  shapef[4] = 9./2. * L0*L1*( 3*L1 - 1. );

  shapef[5] = 9./2. * L1*L2*( 3*L1 - 1. );
  shapef[6] = 9./2. * L1*L2*( 3*L2 - 1. );

  shapef[7] = 9./2. * L2*L0*( 3*L2 - 1. );
  shapef[8] = 9./2. * L2*L0*( 3*L0 - 1. );

  shapef[9] = 27.*L0*L1*L2;
}

////////////////////////////////////////////////////////////////////////////////

void Triag2DLagrangeP3::mapped_coordinates(const CoordsT& coord, const NodeMatrixT& nodes, MappedCoordsT& map_coord)
{
  throw Common::NotImplemented( FromHere(), "" );

  cf_assert(coord.size() == 2);
  cf_assert(map_coord.size() == 2);
  cf_assert(nodes.size() == 6);

  const Real invDet = 1. / jacobian_determinant(nodes);

  map_coord[KSI] = invDet * ((nodes(2, YY) - nodes(0, YY))*coord[XX] + (nodes(0, XX) - nodes(2, XX))*coord[YY] - nodes(0, XX)*nodes(2, YY) + nodes(2, XX)*nodes(0, YY));
  map_coord[ETA] = invDet * ((nodes(0, YY) - nodes(1, YY))*coord[XX] + (nodes(1, XX) - nodes(0, XX))*coord[YY] + nodes(0, XX)*nodes(1, YY) - nodes(1, XX)*nodes(0, YY));

}

////////////////////////////////////////////////////////////////////////////////

void Triag2DLagrangeP3::shape_function_gradient(const MappedCoordsT& map_coord, MappedGradientT& result)
{
  const Real L0 = 1.0 - map_coord[0] - map_coord[1];
  const Real L1 = map_coord[0];
  const Real L2 = map_coord[1];

  const Real dL0dxi  = -1.0;
  const Real dL0deta = -1.0;
  const Real dL1dxi  =  1.0;
  const Real dL1deta =  0.0;
  const Real dL2dxi  =  0.0;
  const Real dL2deta =  1.0;

  result(XX,0) =  0.5 * dL0dxi  * ( 27.0*L0*L0 - 18.0*L0 + 2.0 );
  result(XX,1) =  0.5 * dL1dxi  * ( 27.0*L1*L1 - 18.0*L1 + 2.0 );
  result(XX,2) =  0.5 * dL2dxi  * ( 27.0*L2*L2 - 18.0*L2 + 2.0 );
  result(XX,3) =  4.5 * ( dL0dxi*(6.0*L0*L1-L1) + dL1dxi*L0*(3.0*L0-1.0) );
  result(XX,4) =  4.5 * ( dL1dxi*(6.0*L0*L1-L0) + dL0dxi*L1*(3.0*L1-1.0) );
  result(XX,5) =  4.5 * ( dL1dxi*(6.0*L1*L2-L2) + dL2dxi*L1*(3.0*L1-1.0) );
  result(XX,6) =  4.5 * ( dL2dxi*(6.0*L1*L2-L1) + dL1dxi*L2*(3.0*L2-1.0) );
  result(XX,7) =  4.5 * ( dL2dxi*(6.0*L0*L2-L0) + dL0dxi*L2*(3.0*L2-1.0) );
  result(XX,8) =  4.5 * ( dL0dxi*(6.0*L0*L2-L2) + dL2dxi*L0*(3.0*L0-1.0) );
  result(XX,9) = 27.0 * ( dL0dxi*L1*L2 + L0*dL1dxi*L2 + L0*L1*dL2dxi );

  result(YY,0) =  0.5 * dL0deta  * ( 27.0*L0*L0 - 18.0*L0 + 2.0 );
  result(YY,1) =  0.5 * dL1deta  * ( 27.0*L1*L1 - 18.0*L1 + 2.0 );
  result(YY,2) =  0.5 * dL2deta  * ( 27.0*L2*L2 - 18.0*L2 + 2.0 );
  result(YY,3) =  4.5 * ( dL0deta*(6.0*L0*L1-L1) + dL1deta*L0*(3.0*L0-1.0) );
  result(YY,4) =  4.5 * ( dL1deta*(6.0*L0*L1-L0) + dL0deta*L1*(3.0*L1-1.0) );
  result(YY,5) =  4.5 * ( dL1deta*(6.0*L1*L2-L2) + dL2deta*L1*(3.0*L1-1.0) );
  result(YY,6) =  4.5 * ( dL2deta*(6.0*L1*L2-L1) + dL1deta*L2*(3.0*L2-1.0) );
  result(YY,7) =  4.5 * ( dL2deta*(6.0*L0*L2-L0) + dL0deta*L2*(3.0*L2-1.0) );
  result(YY,8) =  4.5 * ( dL0deta*(6.0*L0*L2-L2) + dL2deta*L0*(3.0*L0-1.0) );
  result(YY,9) = 27.0 * ( dL0deta*L1*L2 + L0*dL1deta*L2 + L0*L1*dL2deta );
}

////////////////////////////////////////////////////////////////////////////////

Real Triag2DLagrangeP3::jacobian_determinant(const MappedCoordsT& map_coord, const NodeMatrixT& nodes)
{
  return jacobian_determinant(nodes);
}

////////////////////////////////////////////////////////////////////////////////

void Triag2DLagrangeP3::jacobian(const MappedCoordsT& map_coord, const NodeMatrixT& nodes, JacobianT& result)
{
  /// @warning only valid for linear geometries of the support

  result(KSI,XX) = nodes(1, XX) - nodes(0, XX);
  result(KSI,YY) = nodes(1, YY) - nodes(0, YY);
  result(ETA,XX) = nodes(2, XX) - nodes(0, XX);
  result(ETA,YY) = nodes(2, YY) - nodes(0, YY);
}

////////////////////////////////////////////////////////////////////////////////

void Triag2DLagrangeP3::jacobian_adjoint(const MappedCoordsT& map_coord, const NodeMatrixT& nodes, JacobianT& result)
{
  throw Common::NotImplemented( FromHere(), "" );

  result(KSI,XX) = nodes(2, YY) - nodes(0, YY);
  result(KSI,YY) = nodes(0, YY) - nodes(1, YY);
  result(ETA,XX) = nodes(0, XX) - nodes(2, XX);
  result(ETA,YY) = nodes(1, XX) - nodes(0, XX);

}

////////////////////////////////////////////////////////////////////////////////

Real Triag2DLagrangeP3::volume(const NodeMatrixT& nodes)
{
  throw Common::NotImplemented( FromHere(), "" );
}

////////////////////////////////////////////////////////////////////////////////

bool Triag2DLagrangeP3::in_element(const CoordsT& coord, const NodeMatrixT& nodes)
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

Real Triag2DLagrangeP3::jacobian_determinant(const NodeMatrixT& nodes)
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
