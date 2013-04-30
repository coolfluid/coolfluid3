// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "mesh/ShapeFunctionT.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Tetra.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP0 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ShapeFunctionT<Tetra>, ShapeFunction, LibLagrangeP0 >
   Tetra_Builder(LibLagrangeP0::library_namespace()+"."+Tetra::type_name());

////////////////////////////////////////////////////////////////////////////////

void Tetra::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  result[0] = 1.;
}

////////////////////////////////////////////////////////////////////////////////

void Tetra::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  result(KSI,0) = 0.;
  result(ETA,0) = 0.;
  result(ZTA,0) = 0.;
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& Tetra::local_coordinates()
{
  static const RealMatrix loc_coord =
      (RealMatrix(nb_nodes, dimensionality) <<

       1./3., 1./3., 1./3.

       ).finished();
  return loc_coord;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP0
} // mesh
} // cf3
