// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "mesh/ShapeFunctionT.hpp"
#include "mesh/LagrangeP2/LibLagrangeP2.hpp"
#include "mesh/LagrangeP2/Line.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP2 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ShapeFunctionT<Line>, ShapeFunction, LibLagrangeP2 >
   Line_Builder(LibLagrangeP2::library_namespace()+"."+Line::type_name());

////////////////////////////////////////////////////////////////////////////////

void Line::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  result[0] = 0.5 * (mapped_coord[KSI]*mapped_coord[KSI] - mapped_coord[KSI]);
  result[1] = 0.5 * (mapped_coord[KSI]*mapped_coord[KSI] + mapped_coord[KSI]);
  result[2] = (1.0 - mapped_coord[KSI]*mapped_coord[KSI]);
}

////////////////////////////////////////////////////////////////////////////////

void Line::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  result(KSI, 0) = mapped_coord[KSI]-0.5;
  result(KSI, 1) = mapped_coord[KSI]+0.5;
  result(KSI, 2) = -2.0*mapped_coord[KSI];
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& Line::local_coordinates()
{
  static const RealMatrix loc_coord =
      (RealMatrix(nb_nodes, dimensionality) <<

       -1.,
        1.,
        0.

       ).finished();
  return loc_coord;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP2
} // mesh
} // cf3
