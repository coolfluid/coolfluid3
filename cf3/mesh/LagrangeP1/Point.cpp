// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "mesh/ShapeFunctionT.hpp"
#include "mesh/LagrangeP1/LibLagrangeP1.hpp"
#include "mesh/LagrangeP1/Point.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ShapeFunctionT<Point>, ShapeFunction, LibLagrangeP1 >
   Point_Builder(LibLagrangeP1::library_namespace()+"."+Point::type_name());

////////////////////////////////////////////////////////////////////////////////

void Point::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  result[0] = 1.;
}

////////////////////////////////////////////////////////////////////////////////

void Point::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  result(KSI,0) = 0.;
  result(ETA,0) = 0.;
  result(ZTA,0) = 0.;
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& Point::local_coordinates()
{
  static const RealMatrix loc_coord =
      (RealMatrix(nb_nodes, 3) <<

       0.,  0., 0.

       ).finished();
  return loc_coord;
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& Point::face_normals()
{
  throw common::NotImplemented(FromHere(),"Point doesn't have faces");
  static const RealMatrix normals;
  return normals;
}

////////////////////////////////////////////////////////////////////////////////

const GeoShape::Type Point::shape;

const Uint Point::nb_nodes;

const Uint Point::nb_faces;

const Uint Point::dimensionality;

const Uint Point::order;

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // mesh
} // cf3
