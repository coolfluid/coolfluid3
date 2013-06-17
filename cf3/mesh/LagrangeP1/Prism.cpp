// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "mesh/ShapeFunctionT.hpp"
#include "mesh/LagrangeP1/LibLagrangeP1.hpp"
#include "mesh/LagrangeP1/Prism.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ShapeFunctionT<Prism>, ShapeFunction, LibLagrangeP1 >
   Prism_Builder(LibLagrangeP1::library_namespace()+"."+Prism::type_name());

////////////////////////////////////////////////////////////////////////////////

void Prism::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  result[0] = 0.5 * (1. - mapped_coord[ZTA]) * (1. - mapped_coord[KSI] - mapped_coord[ETA]);
  result[1] = 0.5 * (1. - mapped_coord[ZTA]) * mapped_coord[KSI];
  result[2] = 0.5 * (1. - mapped_coord[ZTA]) * mapped_coord[ETA];
  result[3] = 0.5 * (1. + mapped_coord[ZTA]) * (1. - mapped_coord[KSI] - mapped_coord[ETA]);
  result[4] = 0.5 * (1. + mapped_coord[ZTA]) * mapped_coord[KSI];
  result[5] = 0.5 * (1. + mapped_coord[ZTA]) * mapped_coord[ETA];
}

////////////////////////////////////////////////////////////////////////////////

void Prism::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  result(KSI, 0) = 0.5 * (mapped_coord[ZTA] - 1.);
  result(ETA, 0) = 0.5 * (mapped_coord[ZTA] - 1.);
  result(ZTA, 0) = 0.5 * (mapped_coord[KSI] + mapped_coord[ETA] - 1.);
  result(KSI, 1) = 0.5 * (1. - mapped_coord[ZTA]);
  result(ETA, 1) = 0.;
  result(ZTA, 1) = -0.5 * mapped_coord[KSI];
  result(KSI, 2) = 0.;
  result(ETA, 2) = 0.5 * (1. - mapped_coord[ZTA]);
  result(ZTA, 2) = -0.5 * mapped_coord[ETA];

  result(KSI, 3) = -0.5 * (mapped_coord[ZTA] + 1.);
  result(ETA, 3) = -0.5 * (mapped_coord[ZTA] + 1.);
  result(ZTA, 3) = 0.5 * (1. - mapped_coord[KSI] - mapped_coord[ETA]);
  result(KSI, 4) = 0.5 * (1. + mapped_coord[ZTA]);
  result(ETA, 4) = 0.;
  result(ZTA, 4) = 0.5 * mapped_coord[KSI];
  result(KSI, 5) = 0.;
  result(ETA, 5) = 0.5 * (1. + mapped_coord[ZTA]);
  result(ZTA, 5) = 0.5 * mapped_coord[ETA];
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& Prism::local_coordinates()
{
  static const RealMatrix loc_coord =
      (RealMatrix(nb_nodes, dimensionality) <<

       0.,  0., -1.,
       1.,  0., -1.,
       0.,  1., -1.,
       0.,  0., 1.,
       1.,  0., 1.,
       0.,  1., 1.

       ).finished();
  return loc_coord;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // mesh
} // cf3
