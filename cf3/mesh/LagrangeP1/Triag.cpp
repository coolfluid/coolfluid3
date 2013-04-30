// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "mesh/ShapeFunctionT.hpp"
#include "mesh/LagrangeP1/LibLagrangeP1.hpp"
#include "mesh/LagrangeP1/Triag.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ShapeFunctionT<Triag>, ShapeFunction, LibLagrangeP1 >
   Triag_Builder(LibLagrangeP1::library_namespace()+"."+Triag::type_name());

////////////////////////////////////////////////////////////////////////////////

void Triag::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  result[0] = 1. - mapped_coord[KSI] - mapped_coord[ETA];
  result[1] = mapped_coord[KSI];
  result[2] = mapped_coord[ETA];
}

////////////////////////////////////////////////////////////////////////////////

void Triag::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  result(KSI, 0) = -1.;
  result(ETA, 0) = -1.;
  result(KSI, 1) =  1.;
  result(ETA, 1) =  0.;
  result(KSI, 2) =  0.;
  result(ETA, 2) =  1.;
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& Triag::local_coordinates()
{
  static const RealMatrix loc_coord =
      (RealMatrix(nb_nodes, dimensionality) <<

       0.,  0.,
       1.,  0.,
       0.,  1.

       ).finished();
  return loc_coord;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // mesh
} // cf3
