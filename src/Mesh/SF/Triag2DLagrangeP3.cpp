// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/BasicExceptions.hpp"
#include "Common/CBuilder.hpp"

#include "LibSF.hpp"
#include "Triag2DLagrangeP3.hpp"
#include "Line2DLagrangeP3.hpp"
#include "SFTriagLagrangeP3.hpp"

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Triag2DLagrangeP3,ElementType,LibSF > aTriag2DLagrangeP3_Builder;

////////////////////////////////////////////////////////////////////////////////

Triag2DLagrangeP3::Triag2DLagrangeP3(const std::string& name) : Triag2D(name)
{
  m_nb_nodes = nb_nodes;
  m_order = order;
}

////////////////////////////////////////////////////////////////////////////////

Real Triag2DLagrangeP3::compute_volume(const NodesT& coord) const
{
  return volume(coord);
}

////////////////////////////////////////////////////////////////////////////////

bool Triag2DLagrangeP3::is_coord_in_element(const RealVector& coord, const NodesT& nodes) const
{
	return in_element(coord,nodes);
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Triag2DLagrangeP3::faces()
{
  throw Common::NotImplemented( FromHere(), "" );

  static FaceConnectivity connectivity;

  if(connectivity.face_first_nodes.empty())
  {
    connectivity.face_first_nodes = boost::assign::list_of(0)(4)(8);
    connectivity.face_node_counts.assign(nb_nodes, 4);
    connectivity.face_nodes = boost::assign::list_of(0)(1)(3)(4)
                                                    (1)(2)(5)(6)
                                                    (2)(0)(7)(8);
  }
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Triag2DLagrangeP3::face_connectivity() const
{
  return faces();
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType& Triag2DLagrangeP3::face_type(const CF::Uint face) const
{
  static const Line2DLagrangeP3 facetype;
  return facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Triag2DLagrangeP3::shape_function_value(const MappedCoordsT& map_coord, ShapeFunctionsT& shapef)
{
  SFTriagLagrangeP3::compute_value(map_coord,shapef);
}

////////////////////////////////////////////////////////////////////////////////

void Triag2DLagrangeP3::mapped_coordinates(const CoordsT& coord, const NodeMatrixT& nodes, MappedCoordsT& map_coord)
{
  throw Common::NotImplemented( FromHere(), "" );

  cf_assert(coord.size() == 2);
  cf_assert(map_coord.size() == 2);
  cf_assert(nodes.size() == 6);

  const Real invDet = 1. / jacobian_determinant(nodes);

  map_coord[KSI] = invDet * ((nodes(2, YY) - nodes(0, YY))*coord[XX] + (nodes(0, XX) - nodes(2, XX))*coord[YY] - nodes(0, XX)*nodes(2, YY) + nodes(2, XX)*nodes(0, YY));
  map_coord[ETA] = invDet * ((nodes(0, YY) - nodes(1, YY))*coord[XX] + (nodes(1, XX) - nodes(0, XX))*coord[YY] + nodes(0, XX)*nodes(1, YY) - nodes(1, XX)*nodes(0, YY));

}

////////////////////////////////////////////////////////////////////////////////

void Triag2DLagrangeP3::shape_function_gradient(const MappedCoordsT& map_coord, MappedGradientT& result)
{
  SFTriagLagrangeP3::compute_gradient(map_coord,result);
}

////////////////////////////////////////////////////////////////////////////////

Real Triag2DLagrangeP3::jacobian_determinant(const MappedCoordsT& map_coord, const NodeMatrixT& nodes)
{
  return jacobian_determinant(nodes);
}

////////////////////////////////////////////////////////////////////////////////

void Triag2DLagrangeP3::jacobian(const MappedCoordsT& map_coord, const NodeMatrixT& nodes, JacobianT& result)
{
  /// @warning only valid for linear geometries of the support

  result(KSI,XX) = nodes(1, XX) - nodes(0, XX);
  result(KSI,YY) = nodes(1, YY) - nodes(0, YY);
  result(ETA,XX) = nodes(2, XX) - nodes(0, XX);
  result(ETA,YY) = nodes(2, YY) - nodes(0, YY);
}

////////////////////////////////////////////////////////////////////////////////

void Triag2DLagrangeP3::jacobian_adjoint(const MappedCoordsT& map_coord, const NodeMatrixT& nodes, JacobianT& result)
{
  throw Common::NotImplemented( FromHere(), "" );

  result(KSI,XX) = nodes(2, YY) - nodes(0, YY);
  result(KSI,YY) = nodes(0, YY) - nodes(1, YY);
  result(ETA,XX) = nodes(0, XX) - nodes(2, XX);
  result(ETA,YY) = nodes(1, XX) - nodes(0, XX);

}

////////////////////////////////////////////////////////////////////////////////

Real Triag2DLagrangeP3::volume(const NodeMatrixT& nodes)
{
  throw Common::NotImplemented( FromHere(), "" );
}

////////////////////////////////////////////////////////////////////////////////

bool Triag2DLagrangeP3::in_element(const CoordsT& coord, const NodeMatrixT& nodes)
{
  throw Common::NotImplemented( FromHere(), "" );

  MappedCoordsT mapped_coord;
  mapped_coordinates(coord, nodes, mapped_coord);
  if( (mapped_coord[KSI] >= -Math::Consts::eps()) &&
      (mapped_coord[ETA] >= -Math::Consts::eps()) &&
      (mapped_coord.sum() <= 1.))
  {
    return true;
  }
  else
  {
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

Real Triag2DLagrangeP3::jacobian_determinant(const NodeMatrixT& nodes)
{
  throw Common::NotImplemented( FromHere(), "" );

  const Real x0 = nodes(0, XX);
  const Real x1 = nodes(1, XX);
  const Real x2 = nodes(2, XX);
  const Real y0 = nodes(0, YY);
  const Real y1 = nodes(1, YY);
  const Real y2 = nodes(2, YY);

  return (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0);
}

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF
