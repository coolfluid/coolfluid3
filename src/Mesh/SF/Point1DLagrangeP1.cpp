// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "Point1DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Point1DLagrangeP1,
                         ElementType,
                         LibSF >
aPoint1DLagrangeP1_Builder;

////////////////////////////////////////////////////////////////////////////////

Point1DLagrangeP1::Point1DLagrangeP1(const std::string& name) : Point1D(name)
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

////////////////////////////////////////////////////////////////////////////////

std::string Point1DLagrangeP1::element_type_name() const
{
  return type_name();
}

////////////////////////////////////////////////////////////////////////////////

Real Point1DLagrangeP1::compute_volume(const NodesT& coord) const
{
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

bool Point1DLagrangeP1::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
{
  return false;
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Point1DLagrangeP1::face_connectivity() const
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

const CF::Mesh::ElementType& Point1DLagrangeP1::face_type(const CF::Uint face) const
{
  static const Point1DLagrangeP1 facetype;
  return facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Point1DLagrangeP1::shape_function(const MappedCoordsT& mappedCoord, ShapeFunctionsT& shapeFunc)
{
  shapeFunc[0] = 1.;
}

////////////////////////////////////////////////////////////////////////////////

void Point1DLagrangeP1::mapped_coordinates(const CoordsT& coord, const NodeMatrixT& nodes, MappedCoordsT& mappedCoord)
{
  mappedCoord[KSI] = 0.;
}

////////////////////////////////////////////////////////////////////////////////

void Point1DLagrangeP1::mapped_gradient(const MappedCoordsT& mappedCoord, MappedGradientT& result)
{
  result(XX, 0) = 0.;
}

////////////////////////////////////////////////////////////////////////////////

void Point1DLagrangeP1::jacobian(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, JacobianT& result)
{
  result(KSI,XX) = 1.;
}

////////////////////////////////////////////////////////////////////////////////

void Point1DLagrangeP1::normal(const MappedCoordsT& mappedCoord, const NodeMatrixT& nodes, CoordsT& result)
{
  result[XX] = 1.;
}

////////////////////////////////////////////////////////////////////////////////

Real Point1DLagrangeP1::area(const NodeMatrixT& nodes)
{
  return 1.;
}

////////////////////////////////////////////////////////////////////////////////

Real Point1DLagrangeP1::volume(const NodeMatrixT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
