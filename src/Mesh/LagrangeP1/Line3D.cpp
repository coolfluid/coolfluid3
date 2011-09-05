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
#include "Mesh/LagrangeP1/Line3D.hpp"
#include "Mesh/LagrangeP0/Point1D.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < ElementTypeT<Line3D>, ElementType , LibLagrangeP1 >
   Line3D_Builder(LibLagrangeP1::library_namespace()+"."+Line3D::type_name());

////////////////////////////////////////////////////////////////////////////////

const ShapeFunctionT<Line3D::SF>& Line3D::shape_function()
{
  const static ShapeFunctionT<SF> shape_function_obj;
  return shape_function_obj;
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType::FaceConnectivity& Line3D::faces()
{
  static ElementType::FaceConnectivity connectivity;
  return connectivity;
}

////////////////////////////////////////////////////////////////////////////////

const CF::Mesh::ElementType& Line3D::face_type(const CF::Uint face)
{
  static const ElementTypeT<Line3D> facetype;
  return facetype;
}

////////////////////////////////////////////////////////////////////////////////

void Line3D::compute_centroid(const NodesT& nodes , CoordsT& centroid)
{
  centroid[XX] = 0.5*(nodes(0,XX)+nodes(1,XX));
  centroid[YY] = 0.5*(nodes(0,YY)+nodes(1,YY));
  centroid[ZZ] = 0.5*(nodes(0,ZZ)+nodes(1,ZZ));
}

////////////////////////////////////////////////////////////////////////////////

Real Line3D::volume(const NodesT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

Real Line3D::area(const NodesT& nodes)
{
  return 0.;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // Mesh
} // CF
