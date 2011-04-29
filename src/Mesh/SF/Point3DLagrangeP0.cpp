// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "Point3DLagrangeP0.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Point3DLagrangeP0,
                         ElementType,
                         LibSF >
aPoint3DLagrangeP0_Builder;

////////////////////////////////////////////////////////////////////////////////

Point3DLagrangeP0::Point3DLagrangeP0(const std::string& name) : Point<DIM_3D,SFPointLagrangeP0>(name)
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

////////////////////////////////////////////////////////////////////////////////

Real Point3DLagrangeP0::compute_volume(const NodesT& coord) const
{
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

Real Point3DLagrangeP0::compute_area(const NodesT& coord) const
{
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

void Point3DLagrangeP0::compute_normal(const NodesT& coord, RealVector& normal) const
{
  throw Common::IllegalCall(FromHere(), "normal of a point in 3D is not defined");
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Point3DLagrangeP0::face_connectivity() const
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

const CF::Mesh::ElementType& Point3DLagrangeP0::face_type(const CF::Uint face) const
{
  static const Point3DLagrangeP0 facetype;
  return facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Point3DLagrangeP0::mapped_coordinates(const CoordsT& coord, const NodeMatrixT& nodes, MappedCoordsT& mappedCoord)
{
  mappedCoord[KSI] = 0.;
}

////////////////////////////////////////////////////////////////////////////////

void Point3DLagrangeP0::jacobian(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, JacobianT& result)
{
  result(KSI,XX) = 1.;
  result(KSI,YY) = 1.;
  result(KSI,ZZ) = 1.;
}

////////////////////////////////////////////////////////////////////////////////

Real Point3DLagrangeP0::jacobian_determinant(const MappedCoordsT& mapped_coord, const NodeMatrixT& nodes)
{
  /// @todo Is this correct? Can the jacobian determinant be calculated from a 1x3 matrix
  return 1.;
}

////////////////////////////////////////////////////////////////////////////////

void Point3DLagrangeP0::jacobian_adjoint(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, JacobianT& result)
{
  /// @todo Is this correct? is the jacobian adjoint a 1x3 matrix?
  result(KSI,XX) = 1.;
  result(KSI,YY) = 1.;
  result(KSI,ZZ) = 1.;
}

////////////////////////////////////////////////////////////////////////////////

void Point3DLagrangeP0::normal(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, CoordsT& result)
{
  throw Common::IllegalCall(FromHere(), "normal of a point in 3D is not defined");
}

////////////////////////////////////////////////////////////////////////////////

Real Point3DLagrangeP0::area(const NodeMatrixT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

Real Point3DLagrangeP0::volume(const NodeMatrixT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
