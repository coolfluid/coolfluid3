// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "Quad3DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Quad3DLagrangeP1, ElementType, LibSF > aQuad3DLagrangeP1_Builder;

////////////////////////////////////////////////////////////////////////////////

Quad3DLagrangeP1::Quad3DLagrangeP1(const std::string& name) : Quad3D(name)
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

////////////////////////////////////////////////////////////////////////////////

Real Quad3DLagrangeP1::compute_volume(const NodesT& coord) const
{
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

void Quad3DLagrangeP1::compute_centroid(const NodesT& coord , RealVector& centroid) const
{
  centroid[0] = 0.25*(coord(0,XX)+coord(1,XX)+coord(2,XX)+coord(3,XX));
  centroid[1] = 0.25*(coord(0,YY)+coord(1,YY)+coord(2,YY)+coord(3,YY));
  centroid[2] = 0.25*(coord(0,ZZ)+coord(1,ZZ)+coord(2,ZZ)+coord(3,ZZ));
}

////////////////////////////////////////////////////////////////////////////////

Real Quad3DLagrangeP1::compute_area(const NodesT& coord) const
{
  return area(coord);
}
////////////////////////////////////////////////////////////////////////////////

bool Quad3DLagrangeP1::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////

const ElementType::FaceConnectivity& Quad3DLagrangeP1::face_connectivity() const
{
  static FaceConnectivity connectivity;
  if(connectivity.face_first_nodes.empty())
  {
    connectivity.face_first_nodes = boost::assign::list_of(0);
    connectivity.face_node_counts.assign(1, 4);
    connectivity.face_nodes = boost::assign::list_of(0)(1)(2)(3);
  }
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType& Quad3DLagrangeP1::face_type(const CF::Uint face) const
{
  static const Quad3DLagrangeP1 facetype;
  return facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Quad3DLagrangeP1::shape_function_value(const MappedCoordsT& mapped_coord, ShapeFunctionsT& shape_func)
{
  SFQuadLagrangeP1::compute_value(mapped_coord, shape_func);
}

////////////////////////////////////////////////////////////////////////////////

void Quad3DLagrangeP1::shape_function_gradient(const MappedCoordsT& mapped_coord, MappedGradientT& result)
{
  SFQuadLagrangeP1::compute_gradient(mapped_coord, result);
}

////////////////////////////////////////////////////////////////////////////////

void Quad3DLagrangeP1::jacobian(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, JacobianT& result)
{
  JacobianCoefficients jc(nodes);

  const Real xi = mappedCoord[KSI];
  const Real eta = mappedCoord[ETA];

  result(KSI,XX) = jc.bx + jc.dx*eta;
  result(KSI,YY) = jc.by + jc.dy*eta;
  result(KSI,ZZ) = jc.bz + jc.dz*eta;

  result(ETA,XX) = jc.cx + jc.dx*xi;
  result(ETA,YY) = jc.cy + jc.dy*xi;
  result(ETA,ZZ) = jc.cz + jc.dz*xi;
}

////////////////////////////////////////////////////////////////////////////////

void Quad3DLagrangeP1::normal(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, CoordsT& result)
{
  JacobianT jac;
  jacobian(mappedCoord, nodes, jac);

  result[XX] = jac(KSI,YY)*jac(ETA,ZZ) - jac(KSI,ZZ)*jac(ETA,YY);
  result[YY] = jac(KSI,ZZ)*jac(ETA,XX) - jac(KSI,XX)*jac(ETA,ZZ);
  result[ZZ] = jac(KSI,XX)*jac(ETA,YY) - jac(KSI,YY)*jac(ETA,XX);
}

////////////////////////////////////////////////////////////////////////////////

Real Quad3DLagrangeP1::volume(const NodeMatrixT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

Real Quad3DLagrangeP1::area(const NodeMatrixT& nodes)
{
  CoordsT n;
  normal(MappedCoordsT::Zero(), nodes, n);
  return 4.*n.norm();
}

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
