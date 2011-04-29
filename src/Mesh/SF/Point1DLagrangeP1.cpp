// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "Point1DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Point1DLagrangeP1,ElementType, LibSF > Point1DLagrangeP1_Builder;

////////////////////////////////////////////////////////////////////////////////

Point1DLagrangeP1::Point1DLagrangeP1(const std::string& name) : Point<DIM_1D,SFPointLagrangeP1>(name)
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

////////////////////////////////////////////////////////////////////////////////

Real Point1DLagrangeP1::compute_volume(const NodesT& coord) const
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

Real Point1DLagrangeP1::compute_area(const NodesT& coord) const
{
  return 1.;
}

////////////////////////////////////////////////////////////////////////////////

void Point1DLagrangeP1::compute_normal(const NodesT& coord, RealVector& normal) const
{
  normal[XX] = 1.;
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Point1DLagrangeP1::face_connectivity() const
{
  return faces();
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType& Point1DLagrangeP1::face_type(const CF::Uint face) const
{
  throw Common::NotImplemented(FromHere(),"");
  const static Point1DLagrangeP1 facetype;
  return facetype;
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Point1DLagrangeP1::faces()
{
  static FaceConnectivity connectivity;
  if(connectivity.face_first_nodes.empty())
  {
    connectivity.face_first_nodes = boost::assign::list_of(0);
    connectivity.face_node_counts.assign(1, 1);
    connectivity.face_nodes = boost::assign::list_of(0);
  }
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

void Point1DLagrangeP1::mapped_coordinates(const CoordsT& coord, const NodeMatrixT& nodes, MappedCoordsT& mappedCoord)
{
  mappedCoord[KSI] = 0;
}

////////////////////////////////////////////////////////////////////////////////

Real Point1DLagrangeP1::jacobian_determinant(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes)
{
  return 1.;
}

////////////////////////////////////////////////////////////////////////////////

void Point1DLagrangeP1::jacobian(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, JacobianT& result)
{
  result(KSI,XX) = 1.;
}

////////////////////////////////////////////////////////////////////////////////

void Point1DLagrangeP1::jacobian_adjoint(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, JacobianT& result)
{
  result(KSI,XX) = 1.;
}

////////////////////////////////////////////////////////////////////////////////

Real Point1DLagrangeP1::volume(const NodeMatrixT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

Real Point1DLagrangeP1::area(const NodeMatrixT& nodes)
{
  return 1.;
}

////////////////////////////////////////////////////////////////////////////////

void Point1DLagrangeP1::normal(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, CoordsT& result)
{
  result[XX] = 1.;
}

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
