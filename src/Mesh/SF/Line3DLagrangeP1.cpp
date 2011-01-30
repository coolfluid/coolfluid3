// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "Line3DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Line3DLagrangeP1, ElementType, LibSF > aLine3DLagrangeP1_Builder;

////////////////////////////////////////////////////////////////////////////////

Line3DLagrangeP1::Line3DLagrangeP1(const std::string& name) : Line3D(name)
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

////////////////////////////////////////////////////////////////////////////////

std::string Line3DLagrangeP1::element_type_name() const
{
  return type_name();
}

////////////////////////////////////////////////////////////////////////////////

Real Line3DLagrangeP1::compute_volume(const NodesT& coord) const
{
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

bool Line3DLagrangeP1::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
{
  return false;
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Line3DLagrangeP1::face_connectivity() const
{
  static FaceConnectivity connectivity;
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType& Line3DLagrangeP1::face_type(const CF::Uint face) const
{
  static const Line3DLagrangeP1 facetype;
  return facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Line3DLagrangeP1::shape_function(const MappedCoordsT& mappedCoord, ShapeFunctionsT& shapeFunc)
{
  shapeFunc[0] = 0.5 * (1.0 - mappedCoord[KSI]);
  shapeFunc[1] = 0.5 * (1.0 + mappedCoord[KSI]);
}

////////////////////////////////////////////////////////////////////////////////

void Line3DLagrangeP1::mapped_gradient(const MappedCoordsT& mappedCoord, MappedGradientT& result)
{
  result(XX, 0) = -0.5;
  result(XX, 1) = 0.5;
}

////////////////////////////////////////////////////////////////////////////////

void Line3DLagrangeP1::jacobian(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, JacobianT& result)
{
  result(KSI,XX) = 0.5*(nodes(1, XX) - nodes(0, XX));
  result(KSI,YY) = 0.5*(nodes(1, YY) - nodes(0, YY));
  result(KSI,ZZ) = 0.5*(nodes(1, ZZ) - nodes(0, ZZ));
}

////////////////////////////////////////////////////////////////////////////////

Real Line3DLagrangeP1::volume(const NodeMatrixT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

Real Line3DLagrangeP1::area(const NodeMatrixT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

Real Line3DLagrangeP1::length(const NodeMatrixT& nodes)
{
  return (nodes.row(1)-nodes.row(0)).norm();
}

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
