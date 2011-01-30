// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "Triag3DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Triag3DLagrangeP1,ElementType,LibSF > aTriag3DLagrangeP1_Builder;

////////////////////////////////////////////////////////////////////////////////

Triag3DLagrangeP1::Triag3DLagrangeP1(const std::string& name) : Triag3D(name)
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

////////////////////////////////////////////////////////////////////////////////

std::string Triag3DLagrangeP1::element_type_name() const
{
  return type_name();
}

////////////////////////////////////////////////////////////////////////////////

Real Triag3DLagrangeP1::compute_volume(const NodesT& coord) const
{
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

bool Triag3DLagrangeP1::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
{
	return false;
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Triag3DLagrangeP1::face_connectivity() const
{
  static FaceConnectivity connectivity;
  if(connectivity.face_first_nodes.empty())
  {
    connectivity.face_first_nodes = boost::assign::list_of(0);
    connectivity.face_node_counts.assign(1, 3);
    connectivity.face_nodes = boost::assign::list_of(0)(1)(2);
  }
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType& Triag3DLagrangeP1::face_type(const CF::Uint face) const
{
  static const Triag3DLagrangeP1 facetype;
  return facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Triag3DLagrangeP1::shape_function(const MappedCoordsT& mapped_coord, ShapeFunctionsT& shape_func)
{
  Triag2DLagrangeP1::shape_function(mapped_coord, shape_func);
}

////////////////////////////////////////////////////////////////////////////////

void Triag3DLagrangeP1::mapped_gradient(const MappedCoordsT& mapped_coord, MappedGradientT& result)
{
  Triag2DLagrangeP1::mapped_gradient(mapped_coord, result);
}

////////////////////////////////////////////////////////////////////////////////

void Triag3DLagrangeP1::jacobian(const MappedCoordsT& mapped_coord, const NodeMatrixT& nodes, JacobianT& result)
{
  const Real x0 = nodes(0, XX);
  const Real x1 = nodes(1, XX);
  const Real x2 = nodes(2, XX);

  const Real y0 = nodes(0, YY);
  const Real y1 = nodes(1, YY);
  const Real y2 = nodes(2, YY);

  const Real z0 = nodes(0, ZZ);
  const Real z1 = nodes(1, ZZ);
  const Real z2 = nodes(2, ZZ);

  result(KSI,XX) = x1 - x0;
  result(KSI,YY) = y1 - y0;
  result(KSI,ZZ) = z1 - z0;

  result(ETA,XX) = x2 - x0;
  result(ETA,YY) = y2 - y0;
  result(ETA,ZZ) = z2 - z0;
}

////////////////////////////////////////////////////////////////////////////////

void Triag3DLagrangeP1::normal(const MappedCoordsT& mapped_coord, const NodeMatrixT& nodes, CoordsT& result)
{
  JacobianT jac;
  jacobian(mapped_coord, nodes, jac);

  result[XX] = jac(KSI,YY)*jac(ETA,ZZ) - jac(KSI,ZZ)*jac(ETA,YY);
  result[YY] = jac(KSI,ZZ)*jac(ETA,XX) - jac(KSI,XX)*jac(ETA,ZZ);
  result[ZZ] = jac(KSI,XX)*jac(ETA,YY) - jac(KSI,YY)*jac(ETA,XX);
}

////////////////////////////////////////////////////////////////////////////////

Real Triag3DLagrangeP1::volume(const NodeMatrixT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

Real Triag3DLagrangeP1::area(const NodeMatrixT& nodes)
{
  CoordsT n;
  normal(MappedCoordsT::Zero(), nodes, n);
  return 0.5*n.norm();
}

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
