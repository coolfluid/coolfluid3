// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Mesh/ShapeFunctionT.hpp"
#include "Mesh/LagrangeP1/Quad.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < ShapeFunctionT<Quad>, ShapeFunction, LibLagrangeP1 >
   Quad_Builder(LibLagrangeP1::library_namespace()+"."+Quad::type_name());

////////////////////////////////////////////////////////////////////////////////

Quad::ValueT Quad::value(const MappedCoordsT& mapped_coord)
{
  ValueT result;
  compute_value(mapped_coord,result);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

Quad::GradientT Quad::gradient(const MappedCoordsT& mapped_coord)
{
  GradientT result;
  compute_gradient(mapped_coord,result);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

void Quad::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  const Real ksi  = mapped_coord[KSI];
  const Real eta = mapped_coord[ETA];

  result[0] = 0.25 * (1.0 - ksi) * (1.0 - eta);
  result[1] = 0.25 * (1.0 + ksi) * (1.0 - eta);
  result[2] = 0.25 * (1.0 + ksi) * (1.0 + eta);
  result[3] = 0.25 * (1.0 - ksi) * (1.0 + eta);
}

////////////////////////////////////////////////////////////////////////////////

void Quad::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  const Real ksi = mapped_coord[KSI];
  const Real eta = mapped_coord[ETA];

  result(KSI, 0) = 0.25 * (-1. + eta);
  result(ETA, 0) = 0.25 * (-1. + ksi);
  result(KSI, 1) = 0.25 * ( 1. - eta);
  result(ETA, 1) = 0.25 * (-1. - ksi);
  result(KSI, 2) = 0.25 * ( 1. + eta);
  result(ETA, 2) = 0.25 * ( 1. + ksi);
  result(KSI, 3) = 0.25 * (-1. - eta);
  result(ETA, 3) = 0.25 * ( 1. - ksi);
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& Quad::local_coordinates()
{
  static const RealMatrix loc_coord =
      (RealMatrix(nb_nodes, dimensionality) <<

       -1., -1.,
        1., -1.,
        1.,  1.,
       -1.,  1.

       ).finished();
  return loc_coord;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // Mesh
} // CF
