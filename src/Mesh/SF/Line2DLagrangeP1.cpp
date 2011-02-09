// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "Line2DLagrangeP1.hpp"
#include "Point1DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Line2DLagrangeP1,
                         ElementType,
                         LibSF >
aLine2DLagrangeP1_Builder;

////////////////////////////////////////////////////////////////////////////////

Line2DLagrangeP1::Line2DLagrangeP1(const std::string& name) : Line2D(name)
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

////////////////////////////////////////////////////////////////////////////////

std::string Line2DLagrangeP1::element_type_name() const
{
  return type_name();
}

////////////////////////////////////////////////////////////////////////////////

Real Line2DLagrangeP1::compute_volume(const NodesT& coord) const
{
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

Real Line2DLagrangeP1::compute_area(const NodesT& coord) const
{
  return area(coord);
}

////////////////////////////////////////////////////////////////////////////////

void Line2DLagrangeP1::compute_normal(const NodesT& coord, RealVector& normal) const
{
  normal[XX] = -coord(0,YY) + coord(1,YY);
  normal[YY] = -coord(1,XX) + coord(0,XX);
  normal.normalize();
}

////////////////////////////////////////////////////////////////////////////////

bool Line2DLagrangeP1::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
{
  return false;
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Line2DLagrangeP1::face_connectivity() const
{
  static FaceConnectivity connectivity;
  if(connectivity.face_first_nodes.empty())
  {
    connectivity.face_first_nodes = boost::assign::list_of(0);
    connectivity.face_node_counts.assign(1, 2);
    connectivity.face_nodes = boost::assign::list_of(0)(1);
  }
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType& Line2DLagrangeP1::face_type(const CF::Uint face) const
{
  static const Point1DLagrangeP1 facetype;
  return facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Line2DLagrangeP1::shape_function(const MappedCoordsT& mappedCoord, ShapeFunctionsT& shapeFunc)
{
  shapeFunc[0] = 0.5 * (1.0 - mappedCoord[KSI]);
  shapeFunc[1] = 0.5 * (1.0 + mappedCoord[KSI]);
}

////////////////////////////////////////////////////////////////////////////////

void Line2DLagrangeP1::mapped_gradient(const MappedCoordsT& mappedCoord, MappedGradientT& result)
{
  result(XX, 0) = -0.5;
  result(XX, 1) = 0.5;
}

////////////////////////////////////////////////////////////////////////////////

void Line2DLagrangeP1::jacobian(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, JacobianT& result)
{
  result(KSI,XX) = 0.5*(nodes(1, XX) - nodes(0, XX));
  result(KSI,YY) = 0.5*(nodes(1, YY) - nodes(0, YY));
}

////////////////////////////////////////////////////////////////////////////////

void Line2DLagrangeP1::normal(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, CoordsT& result)
{
  result[XX] = 0.5*( nodes(1, YY) - nodes(0, YY));
  result[YY] = 0.5*(-nodes(1, XX) + nodes(0, XX));
}

////////////////////////////////////////////////////////////////////////////////

Real Line2DLagrangeP1::volume(const NodeMatrixT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

Real Line2DLagrangeP1::area(const NodeMatrixT& nodes)
{
  return (nodes.row(1)-nodes.row(0)).norm();
}

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
