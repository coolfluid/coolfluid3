// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/CBuilder.hpp"

#include "Math/Consts.hpp"

#include "Mesh/ElementTypeT.hpp"
#include "Mesh/ShapeFunctionT.hpp"

#include "Mesh/LagrangeP1/LibLagrangeP1.hpp"
#include "Mesh/LagrangeP1/Line2D.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < ElementTypeT<Line2D>, ElementType , LibLagrangeP1 >
   Line2D_Builder(LibLagrangeP1::library_namespace()+"."+Line2D::type_name());

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Line2D::faces()
{
  static ElementType::FaceConnectivity connectivity;
  if(connectivity.displs.empty())
  {
    connectivity.displs = boost::assign::list_of(0);
    connectivity.stride.assign(1, 2);
    connectivity.nodes = boost::assign::list_of(0)(1);
  }
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType& Line2D::face_type(const CF::Uint face)
{
  static const ElementType::ConstPtr facetype( Common::allocate_component<ElementTypeT<Line2D> >(Line2D::type_name()) );
  return *facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Line2D::compute_centroid(const NodesT& nodes , CoordsT& centroid)
{
  centroid[XX] = 0.5*(nodes(0,XX)+nodes(1,XX));
  centroid[YY] = 0.5*(nodes(0,YY)+nodes(1,YY));
}

////////////////////////////////////////////////////////////////////////////////

void Line2D::compute_normal(const NodesT& nodes , CoordsT& result)
{
  result[XX] = 0.5*( nodes(1, YY) - nodes(0, YY));
  result[YY] = 0.5*(-nodes(1, XX) + nodes(0, XX));
  result.normalize();
}

////////////////////////////////////////////////////////////////////////////////

Real Line2D::volume(const NodesT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

Real Line2D::area(const NodesT& nodes)
{
  return (nodes.row(1) - nodes.row(0)).norm();
}

////////////////////////////////////////////////////////////////////////////////

void Line2D::normal(const MappedCoordsT& mapped_coord, const NodesT& nodes, CoordsT& result)
{
  result[XX] = 0.5*( nodes(1, YY) - nodes(0, YY));
  result[YY] = 0.5*(-nodes(1, XX) + nodes(0, XX));
}

////////////////////////////////////////////////////////////////////////////////

void Line2D::compute_jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes, JacobianT& result)
{
  result(KSI,XX) = 0.5*(nodes(1, XX) - nodes(0, XX));
  result(KSI,YY) = 0.5*(nodes(1, YY) - nodes(0, YY));
}

////////////////////////////////////////////////////////////////////////////////

Line2D::JacobianT Line2D::jacobian(const MappedCoordsT& mapped_coord, const NodesT& nodes)
{
  JacobianT result;
  compute_jacobian(mapped_coord, nodes, result);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // Mesh
} // CF
