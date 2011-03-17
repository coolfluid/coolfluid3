// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "Line2DLagrangeP3.hpp"
#include "Quad2DLagrangeP3.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Quad2DLagrangeP3,
                         ElementType,
                         LibSF >
aQuad2DLagrangeP3_Builder;

////////////////////////////////////////////////////////////////////////////////

Quad2DLagrangeP3::Quad2DLagrangeP3(const std::string& name) : Quad2D(name)
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

////////////////////////////////////////////////////////////////////////////////

std::string Quad2DLagrangeP3::element_type_name() const
{
  return type_name();
}

////////////////////////////////////////////////////////////////////////////////

Real Quad2DLagrangeP3::compute_volume(const NodesT& coord) const
{
  return volume(coord);
}

////////////////////////////////////////////////////////////////////////////////

bool Quad2DLagrangeP3::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
{
  // @todo: this was copied & pasted from P1Quad code
  // make sure it works in P3 case as well
  MappedCoordsT mapped_coord;
  mapped_coordinates(CoordsT(coord), nodes, mapped_coord);
  if( (mapped_coord[KSI] >= -1.0) &&
      (mapped_coord[ETA] >= -1.0) &&
      (mapped_coord[KSI] <=  1.0) &&
      (mapped_coord[ETA] <=  1.0))
  {
    return true;
  }
  else
  {
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Quad2DLagrangeP3::faces()
{
  static FaceConnectivity connectivity;
  if(connectivity.face_first_nodes.empty())
  {
    connectivity.face_first_nodes = boost::assign::list_of(0)(4)(8)(12);
    connectivity.face_node_counts.assign(4, 4);
    connectivity.face_nodes = boost::assign::list_of(0)(4)(5)(1)
                                                    (1)(6)(7)(2)
                                                    (2)(8)(9)(3)
                                                    (3)(10)(11)(0);
  }
  return connectivity;
}
////////////////////////////////////////////////////////////////////////////////
// Local connectivity:
//             3---9---8---2
//             |           |
//						10  15   14  7
//             |           |
//						11  12   13  6
//             |           |
//             0---4---5---1
// Reference domain: <-1,1> x <-1,1>

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Quad2DLagrangeP3::face_connectivity() const
{
  return faces();
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType& Quad2DLagrangeP3::face_type(const CF::Uint face) const
{
  const static Line2DLagrangeP3 facetype;
  return facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Quad2DLagrangeP3::shape_function(const MappedCoordsT& mappedCoord, ShapeFunctionsT& shapeFunc)
{
  const Real xi  = mappedCoord[KSI];
  const Real eta = mappedCoord[ETA];

  const Real onethird = 1.0/3.0;

  const Real L0xi  = - 9.0/16.0 * (xi+onethird) * (xi-onethird) * (xi-1.0);
  const Real L1xi  =  27.0/16.0 *    (xi+1.0)   * (xi-onethird) * (xi-1.0);
  const Real L2xi  = -27.0/16.0 *    (xi+1.0)   * (xi+onethird) * (xi-1.0);
  const Real L3xi  =   9.0/16.0 *    (xi+1.0)   * (xi+onethird) * (xi-onethird);

  const Real L0eta = - 9.0/16.0 * (eta+onethird) * (eta-onethird) * (eta-1.0);
  const Real L1eta =  27.0/16.0 *    (eta+1.0)   * (eta-onethird) * (eta-1.0);
  const Real L2eta = -27.0/16.0 *    (eta+1.0)   * (eta+onethird) * (eta-1.0);
  const Real L3eta =   9.0/16.0 *    (eta+1.0)   * (eta+onethird) * (eta-onethird);

  shapeFunc[0]  = L0xi * L0eta;
  shapeFunc[1]  = L3xi * L0eta;
  shapeFunc[2]  = L3xi * L3eta;
  shapeFunc[3]  = L0xi * L3eta;
  shapeFunc[4]  = L1xi * L0eta;
  shapeFunc[5]  = L2xi * L0eta;
  shapeFunc[6]  = L3xi * L1eta;
  shapeFunc[7]  = L3xi * L2eta;
  shapeFunc[8]  = L2xi * L3eta;
  shapeFunc[9]  = L1xi * L3eta;
  shapeFunc[10] = L0xi * L2eta;
  shapeFunc[11] = L0xi * L1eta;
  shapeFunc[12] = L1xi * L1eta;
  shapeFunc[13] = L2xi * L1eta;
  shapeFunc[14] = L2xi * L2eta;
  shapeFunc[15] = L1xi * L2eta;
}

////////////////////////////////////////////////////////////////////////////////

void Quad2DLagrangeP3::mapped_coordinates(const CoordsT& coord, const NodeMatrixT& nodes, MappedCoordsT& mappedCoord)
{
  throw Common::NotImplemented( FromHere(), "" );
}

////////////////////////////////////////////////////////////////////////////////

void Quad2DLagrangeP3::mapped_gradient(const MappedCoordsT& mappedCoord, MappedGradientT& result)
{
  const Real xi =  mappedCoord[KSI];
  const Real eta = mappedCoord[ETA];

  const Real onethird = 1.0/3.0;

  const Real L0xi  = - 9.0/16.0 * (xi+onethird) * (xi-onethird) * (xi-1.0);
  const Real L1xi  =  27.0/16.0 *    (xi+1.0)   * (xi-onethird) * (xi-1.0);
  const Real L2xi  = -27.0/16.0 *    (xi+1.0)   * (xi+onethird) * (xi-1.0);
  const Real L3xi  =   9.0/16.0 *    (xi+1.0)   * (xi+onethird) * (xi-onethird);

  const Real L0eta = - 9.0/16.0 * (eta+onethird) * (eta-onethird) * (eta-1.0);
  const Real L1eta =  27.0/16.0 *    (eta+1.0)   * (eta-onethird) * (eta-1.0);
  const Real L2eta = -27.0/16.0 *    (eta+1.0)   * (eta+onethird) * (eta-1.0);
  const Real L3eta =   9.0/16.0 *    (eta+1.0)   * (eta+onethird) * (eta-onethird);

  const Real dL0dxi = - 9.0/16.0 * (   (xi-onethird)*(xi-1.0)    + (xi+onethird)*(xi-1.0) + (xi+onethird)*(xi-onethird) );
  const Real dL1dxi =  27.0/16.0 * (   (xi-onethird)*(xi-1.0)    +    (xi+1.0)*(xi-1.0)   +   (xi+1.0)*(xi-onethird) );
  const Real dL2dxi = -27.0/16.0 * (   (xi+onethird)*(xi-1.0)    +    (xi+1.0)*(xi-1.0)   +   (xi+1.0)*(xi+onethird) );
  const Real dL3dxi =   9.0/16.0 * ( (xi+onethird)*(xi-onethird) + (xi+1.0)*(xi-onethird) +   (xi+1.0)*(xi+onethird) );

  const Real dL0deta = - 9.0/16.0 * (   (eta-onethird)*(eta-1.0)    + (eta+onethird)*(eta-1.0) + (eta+onethird)*(eta-onethird) );
  const Real dL1deta =  27.0/16.0 * (   (eta-onethird)*(eta-1.0)    +    (eta+1.0)*(eta-1.0)   +   (eta+1.0)*(eta-onethird) );
  const Real dL2deta = -27.0/16.0 * (   (eta+onethird)*(eta-1.0)    +    (eta+1.0)*(eta-1.0)   +   (eta+1.0)*(eta+onethird) );
  const Real dL3deta =   9.0/16.0 * ( (eta+onethird)*(eta-onethird) + (eta+1.0)*(eta-onethird) +   (eta+1.0)*(eta+onethird) );

  result(KSI,0)  = dL0dxi * L0eta;
  result(KSI,1)  = dL3dxi * L0eta;
  result(KSI,2)  = dL3dxi * L3eta;
  result(KSI,3)  = dL0dxi * L3eta;
  result(KSI,4)  = dL1dxi * L0eta;
  result(KSI,5)  = dL2dxi * L0eta;
  result(KSI,6)  = dL3dxi * L1eta;
  result(KSI,7)  = dL3dxi * L2eta;
  result(KSI,8)  = dL2dxi * L3eta;
  result(KSI,9)  = dL1dxi * L3eta;
  result(KSI,10) = dL0dxi * L2eta;
  result(KSI,11) = dL0dxi * L1eta;
  result(KSI,12) = dL1dxi * L1eta;
  result(KSI,13) = dL2dxi * L1eta;
  result(KSI,14) = dL2dxi * L2eta;
  result(KSI,15) = dL1dxi * L2eta;

  result(ETA,0)  = L0xi * dL0deta;
  result(ETA,1)  = L3xi * dL0deta;
  result(ETA,2)  = L3xi * dL3deta;
  result(ETA,3)  = L0xi * dL3deta;
  result(ETA,4)  = L1xi * dL0deta;
  result(ETA,5)  = L2xi * dL0deta;
  result(ETA,6)  = L3xi * dL1deta;
  result(ETA,7)  = L3xi * dL2deta;
  result(ETA,8)  = L2xi * dL3deta;
  result(ETA,9)  = L1xi * dL3deta;
  result(ETA,10) = L0xi * dL2deta;
  result(ETA,11) = L0xi * dL1deta;
  result(ETA,12) = L1xi * dL1deta;
  result(ETA,13) = L2xi * dL1deta;
  result(ETA,14) = L2xi * dL2deta;
  result(ETA,15) = L1xi * dL2deta;
}

////////////////////////////////////////////////////////////////////////////////

Real Quad2DLagrangeP3::jacobian_determinant(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes)
{
  cf_assert(mappedCoord.size() == dimension);
  cf_assert(nodes.size() == nb_nodes);

  throw Common::NotImplemented( FromHere(), "" );
}

////////////////////////////////////////////////////////////////////////////////

void Quad2DLagrangeP3::jacobian(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, JacobianT& result)
{
  throw Common::NotImplemented( FromHere(), "" );
}

////////////////////////////////////////////////////////////////////////////////

void Quad2DLagrangeP3::jacobian_adjoint(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, JacobianT& result)
{
  throw Common::NotImplemented( FromHere(), "" );
}

////////////////////////////////////////////////////////////////////////////////

Real Quad2DLagrangeP3::volume(const NodeMatrixT& nodes)
{
  throw Common::NotImplemented( FromHere(), "" );
}

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
