// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "Line2DLagrangeP1.hpp"
#include "Point2DLagrangeP0.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Line2DLagrangeP1,ElementType, LibSF > Line2DLagrangeP1_Builder;

////////////////////////////////////////////////////////////////////////////////

Line2DLagrangeP1::Line2DLagrangeP1(const std::string& name) : Line<DIM_2D,SFLineLagrangeP1>(name)
{
}

////////////////////////////////////////////////////////////////////////////////

Real Line2DLagrangeP1::compute_volume(const NodesT& coord) const
{
  return 0.;
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

void Line2DLagrangeP1::compute_centroid(const NodesT& coord , RealVector& centroid) const
{
  centroid[XX] = 0.5*(coord(0,XX)+coord(1,XX));
  centroid[YY] = 0.5*(coord(0,YY)+coord(1,YY));
}

////////////////////////////////////////////////////////////////////////////////

bool Line2DLagrangeP1::is_coord_in_element( const RealVector& coord, const NodesT& nodes) const
{
  MappedCoordsT mapped_coord;
  mapped_coordinates(CoordsT(coord), nodes, mapped_coord);
  if( (mapped_coord[KSI] >= -0.5) &&
      (mapped_coord[KSI] <= 0.5) )
  {
    return true;
  }
  else
  {
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Line2DLagrangeP1::face_connectivity() const
{
  return faces();
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType& Line2DLagrangeP1::face_type(const CF::Uint face) const
{
  const static Point2DLagrangeP0 facetype;
  return facetype;
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Line2DLagrangeP1::faces()
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

void Line2DLagrangeP1::mapped_coordinates(const CoordsT& coord, const NodeMatrixT& nodes, MappedCoordsT& mappedCoord)
{
  const Real x0 = nodes(0, XX);
  const Real x1 = nodes(1, XX);
  mappedCoord[KSI] = (2*coord[0] - (x1 + x0)) / (x1 - x0);
}

////////////////////////////////////////////////////////////////////////////////

void Line2DLagrangeP1::jacobian(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, JacobianT& result)
{
  result(KSI,XX) = 0.5*(nodes(1, XX) - nodes(0, XX));
  result(KSI,YY) = 0.5*(nodes(1, YY) - nodes(0, YY));
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

void Line2DLagrangeP1::normal(const MappedCoordsT& mapped_coord, const NodeMatrixT& nodes, CoordsT& result)
{
  result[XX] = 0.5*( nodes(1, YY) - nodes(0, YY));
  result[YY] = 0.5*(-nodes(1, XX) + nodes(0, XX));
}

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
