// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "Line1DLagrangeP1.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Line1DLagrangeP1,ElementType, LibSF >
aLine1DLagrangeP1_Builder;

////////////////////////////////////////////////////////////////////////////////

Line1DLagrangeP1::Line1DLagrangeP1(const std::string& name) : Line1D(name)
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

////////////////////////////////////////////////////////////////////////////////

std::string Line1DLagrangeP1::element_type_name() const
{
  return type_name();
}

////////////////////////////////////////////////////////////////////////////////

Real Line1DLagrangeP1::compute_volume(const NodesT& coord) const
{
  return volume(coord);
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
  static FaceConnectivity connectivity;
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType& Line1DLagrangeP1::face_type(const CF::Uint face) const
{
  // TODO: Add a Point1DLagrangeP1 type to complete this
  throw Common::NotImplemented(FromHere(), "Line1DLagrangeP1::face_type requires a point type");
}

////////////////////////////////////////////////////////////////////////////////

void Line1DLagrangeP1::shape_function(const MappedCoordsT& mappedCoord, ShapeFunctionsT& shapeFunc)
{
  shapeFunc[0] = 0.5 * (1.0 - mappedCoord[KSI]);
  shapeFunc[1] = 0.5 * (1.0 + mappedCoord[KSI]);
}

////////////////////////////////////////////////////////////////////////////////

void Line1DLagrangeP1::mapped_coordinates(const CoordsT& coord, const NodesT& nodes, MappedCoordsT& mappedCoord)
{
  const Real x0 = nodes(0, XX);
  const Real x1 = nodes(1, XX);
  mappedCoord[KSI] = (2*coord[0] - (x1 + x0)) / (x1 - x0);
}

////////////////////////////////////////////////////////////////////////////////

void Line1DLagrangeP1::mapped_gradient(const MappedCoordsT& mappedCoord, MappedGradientT& result)
{
  result(XX, 0) = -0.5;
  result(XX, 1) = 0.5;
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

} // SF
} // Mesh
} // CF
