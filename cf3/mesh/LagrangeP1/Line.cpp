// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "mesh/ShapeFunctionT.hpp"
#include "mesh/LagrangeP1/LibLagrangeP1.hpp"
#include "mesh/LagrangeP1/Line.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ShapeFunctionT<Line>, ShapeFunction, LibLagrangeP1 >
   Line_Builder(LibLagrangeP1::library_namespace()+"."+Line::type_name());

////////////////////////////////////////////////////////////////////////////////

void Line::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  result[0] = 0.5 * (1.0 - mapped_coord[KSI]);
  result[1] = 0.5 * (1.0 + mapped_coord[KSI]);
}

////////////////////////////////////////////////////////////////////////////////

void Line::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  result(KSI, 0) = -0.5;
  result(KSI, 1) =  0.5;
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& Line::local_coordinates()
{
  static const RealMatrix loc_coord =
      (RealMatrix(nb_nodes, dimensionality) <<

       -1.,
        1.

       ).finished();
  return loc_coord;
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& Line::mononomial_coefficients()
{
  static const RealMatrix coeffs=
      (RealMatrix(nb_nodes, nb_nodes) <<

       0.5, -0.5,
       0.5,  0.5

       ).finished();
  return coeffs;
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& Line::mononomial_exponents()
{
  static const RealMatrix exponents=
      (RealMatrix(nb_nodes, dimensionality) <<

       0,
       1

       ).finished();
  return exponents;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // mesh
} // cf3
