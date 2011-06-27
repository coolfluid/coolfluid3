// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "Line1DLagrangeP1.hpp"
#include "Point1DLagrangeP0.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Line1DLagrangeP1,ElementType, LibSF > Line1DLagrangeP1_Builder;

////////////////////////////////////////////////////////////////////////////////

Line1DLagrangeP1::Line1DLagrangeP1(const std::string& name) : Line<DIM_1D,SFLineLagrangeP1>(name)
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

////////////////////////////////////////////////////////////////////////////////

Real Line1DLagrangeP1::compute_volume(const NodesT& coord) const
{
  return volume(coord);
}

////////////////////////////////////////////////////////////////////////////////

Real Line1DLagrangeP1::compute_area(const NodesT& coord) const
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

void Line1DLagrangeP1::compute_normal(const NodesT& coord, RealVector& normal) const
{
  throw Common::IllegalCall(FromHere(),"Normal is not defined for a line in 1D");
}

////////////////////////////////////////////////////////////////////////////////

void Line1DLagrangeP1::compute_centroid(const NodesT& coord , RealVector& centroid) const
{
  centroid[0] = 0.5*(coord(0,XX)+coord(1,XX));
}

////////////////////////////////////////////////////////////////////////////////

bool Line1DLagrangeP1::is_coord_in_element( const RealVector& coord, const NodesT& nodes) const
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

const CF::Mesh::ElementType::FaceConnectivity& Line1DLagrangeP1::face_connectivity() const
{
  return faces();
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType& Line1DLagrangeP1::face_type(const CF::Uint face) const
{
  const static Point1DLagrangeP0 facetype;
  return facetype;
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Line1DLagrangeP1::faces()
{
  static FaceConnectivity connectivity;
  if(connectivity.face_first_nodes.empty())
  {
    connectivity.face_first_nodes = boost::assign::list_of(0)(1);
    connectivity.face_node_counts.assign(2, 1);
    connectivity.face_nodes = boost::assign::list_of(0)
                                                    (1);
  }
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

void Line1DLagrangeP1::mapped_coordinates(const CoordsT& coord, const NodeMatrixT& nodes, MappedCoordsT& mappedCoord)
{
  const Real x0 = nodes(0, XX);
  const Real x1 = nodes(1, XX);
  mappedCoord[KSI] = (2*coord[0] - (x1 + x0)) / (x1 - x0);
}

////////////////////////////////////////////////////////////////////////////////

Real Line1DLagrangeP1::jacobian_determinant(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes)
{
  return 0.5*volume(nodes);
}

////////////////////////////////////////////////////////////////////////////////

void Line1DLagrangeP1::jacobian(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, JacobianT& result)
{
  result(KSI,XX) = jacobian_determinant(mappedCoord, nodes);
}

////////////////////////////////////////////////////////////////////////////////

void Line1DLagrangeP1::jacobian_adjoint(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, JacobianT& result)
{
  result(KSI,XX) = 1.;
}

////////////////////////////////////////////////////////////////////////////////

RealVector Line1DLagrangeP1::plane_jacobian_normal(const RealVector& mapped_coords,
                                                   const RealMatrix& nodes,
                                                   const CoordRef orientation) const
{
  RealVector result(1);
  result[XX] = 1.;
  return result;
}

////////////////////////////////////////////////////////////////////////////////

Real Line1DLagrangeP1::volume(const NodeMatrixT& nodes)
{
  return std::abs(nodes(1, XX) - nodes(0, XX));
}

////////////////////////////////////////////////////////////////////////////////

Real Line1DLagrangeP1::area(const NodeMatrixT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

Real Line1DLagrangeP1::jacobian_determinant(const RealVector& mapped_coord, const RealMatrix& nodes) const
{
  return Line1DLagrangeP1::jacobian_determinant(static_cast<MappedCoordsT>(mapped_coord),static_cast<NodeMatrixT>(nodes));
}

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
