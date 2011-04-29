// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "Line3DLagrangeP1.hpp"
#include "Point3DLagrangeP0.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Line3DLagrangeP1, ElementType, LibSF > aLine3DLagrangeP1_Builder;

////////////////////////////////////////////////////////////////////////////////

Line3DLagrangeP1::Line3DLagrangeP1(const std::string& name) : Line<DIM_3D,SFLineLagrangeP1>(name)
{
}

////////////////////////////////////////////////////////////////////////////////

Real Line3DLagrangeP1::compute_volume(const NodesT& coord) const
{
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

Real Line3DLagrangeP1::compute_area(const NodesT& coord) const
{
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

void Line3DLagrangeP1::compute_normal(const NodesT& coord, RealVector& normal) const
{
  throw Common::IllegalCall(FromHere(),"Normal is not defined for a line in 3D");
}

////////////////////////////////////////////////////////////////////////////////

bool Line3DLagrangeP1::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
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

void Line3DLagrangeP1::compute_centroid(const NodesT& coord , RealVector& centroid) const
{
  centroid = 0.5*(coord.row(0)+coord.row(1));
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
  static const Point3DLagrangeP0 facetype;
  return facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Line3DLagrangeP1::mapped_coordinates(const CoordsT& coord, const NodeMatrixT& nodes, MappedCoordsT& mappedCoord)
{
  throw Common::NotImplemented(FromHere(),"Mapped coordinates computation not implemented for Line3DLagrangeP1 yet. Feel free");
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
