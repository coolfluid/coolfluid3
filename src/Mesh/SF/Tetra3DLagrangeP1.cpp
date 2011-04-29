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
aTetra3DLagrangeP1_Builder;

////////////////////////////////////////////////////////////////////////////////

Tetra3DLagrangeP1::Tetra3DLagrangeP1(const std::string& name) : Tetra3D(name)
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

////////////////////////////////////////////////////////////////////////////////

Real Tetra3DLagrangeP1::compute_volume(const NodesT& coord) const
{
  return volume(coord);
}

////////////////////////////////////////////////////////////////////////////////

void Tetra3DLagrangeP1::compute_centroid(const NodesT& coord , RealVector& centroid) const
{
  centroid[XX] = 0.25*(coord(0,XX)+coord(1,XX)+coord(2,XX)+coord(3,XX));
  centroid[YY] = 0.25*(coord(0,YY)+coord(1,YY)+coord(2,YY)+coord(3,YY));
  centroid[ZZ] = 0.25*(coord(0,ZZ)+coord(1,ZZ)+coord(2,ZZ)+coord(3,ZZ));
}

////////////////////////////////////////////////////////////////////////////////

bool Tetra3DLagrangeP1::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
{
  return in_element(coord,nodes);
}

////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Tetra3DLagrangeP1::face_connectivity() const
{
  return faces();
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType& Tetra3DLagrangeP1::face_type(const CF::Uint face) const
{
  const static Triag3DLagrangeP1 facetype;
  return facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Tetra3DLagrangeP1::shape_function_value(const MappedCoordsT& mappedCoord, ShapeFunctionsT& shapeFunc)
{
  SFTetraLagrangeP1::value(mappedCoord,shapeFunc);
}

////////////////////////////////////////////////////////////////////////////////

void Tetra3DLagrangeP1::mapped_coordinates(const CoordsT& coord, const NodeMatrixT& nodes, MappedCoordsT& mappedCoord)
{
  RealMatrix3 M;
  M.col(0) = nodes.row(1) - nodes.row(0);
  M.col(1) = nodes.row(2) - nodes.row(0);
  M.col(2) = nodes.row(3) - nodes.row(0);
  
  mappedCoord = M.inverse() * (coord - nodes.row(0).transpose());
}

////////////////////////////////////////////////////////////////////////////////
void Tetra3DLagrangeP1::shape_function_gradient(const MappedCoordsT& mappedCoord, MappedGradientT& result)
{
  SFTetraLagrangeP1::gradient(mappedCoord,result);
}

////////////////////////////////////////////////////////////////////////////////

Real Tetra3DLagrangeP1::jacobian_determinant(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes)
{
  return jacobian_determinant(nodes);
}

////////////////////////////////////////////////////////////////////////////////

void Tetra3DLagrangeP1::jacobian(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, JacobianT& result)
{
  const Real x0 = nodes(0, XX);
  const Real y0 = nodes(0, YY);
  const Real z0 = nodes(0, ZZ);

  const Real x1 = nodes(1, XX);
  const Real y1 = nodes(1, YY);
  const Real z1 = nodes(1, ZZ);

  const Real x2 = nodes(2, XX);
  const Real y2 = nodes(2, YY);
  const Real z2 = nodes(2, ZZ);

  const Real x3 = nodes(3, XX);
  const Real y3 = nodes(3, YY);
  const Real z3 = nodes(3, ZZ);

  const Real dxdksi = -x0 + x1;
  const Real dydksi = -y0 + y1;
  const Real dzdksi = -z0 + z1;

  const Real dxdeta = -x0 + x2;
  const Real dydeta = -y0 + y2;
  const Real dzdeta = -z0 + z2;

  const Real dxdzta = -x0 + x3;
  const Real dydzta = -y0 + y3;
  const Real dzdzta = -z0 + z3;

  // Derivatives of shape functions are constant
  // hence Jacobians are independent of the mappedCoord
  result(KSI,XX) = dxdksi;
  result(KSI,YY) = dydksi;
  result(KSI,ZZ) = dzdksi;

  result(ETA,XX) = dxdeta;
  result(ETA,YY) = dydeta;
  result(ETA,ZZ) = dzdeta;

  result(ZTA,XX) = dxdzta;
  result(ZTA,YY) = dydzta;
  result(ZTA,ZZ) = dzdzta;
}

////////////////////////////////////////////////////////////////////////////////

void Tetra3DLagrangeP1::jacobian_adjoint(const MappedCoordsT& mapped_coord, const NodeMatrixT& nodes, JacobianT& result)
{
  JacobianT J;
  jacobian(mapped_coord, nodes, J);
  result(0, 0) =  (J(1, 1)*J(2, 2) - J(1, 2)*J(2, 1));
  result(0, 1) = -(J(0, 1)*J(2, 2) - J(0, 2)*J(2, 1));
  result(0, 2) =  (J(0, 1)*J(1, 2) - J(1, 1)*J(0, 2));
  result(1, 0) = -(J(1, 0)*J(2, 2) - J(1, 2)*J(2, 0));
  result(1, 1) =  (J(0, 0)*J(2, 2) - J(0, 2)*J(2, 0));
  result(1, 2) = -(J(0, 0)*J(1, 2) - J(0, 2)*J(1, 0));
  result(2, 0) =  (J(1, 0)*J(2, 1) - J(1, 1)*J(2, 0));
  result(2, 1) = -(J(0, 0)*J(2, 1) - J(0, 1)*J(2, 0));
  result(2, 2) =  (J(0, 0)*J(1, 1) - J(0, 1)*J(1, 0));
}

////////////////////////////////////////////////////////////////////////////////

Real Tetra3DLagrangeP1::volume(const NodeMatrixT& nodes)
{
  return jacobian_determinant(nodes) / 6.;
}

////////////////////////////////////////////////////////////////////////////////

bool Tetra3DLagrangeP1::in_element(const CoordsT& coord, const NodeMatrixT& nodes)
{
  MappedCoordsT mapped_coord;
  mapped_coordinates(coord, nodes, mapped_coord);
  const Real tolerance = 10*Math::MathConsts::eps();
  if((mapped_coord[KSI] >= -tolerance) &&
     (mapped_coord[ETA] >= -tolerance) &&
     (mapped_coord[ZTA] >= -tolerance) &&
     (mapped_coord.sum() <= 1.+tolerance))
  {
    return true;
  }
  else
  {
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
