// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "mesh/ShapeFunctionT.hpp"
#include "mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "mesh/LagrangeP0/Prism.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP0 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ShapeFunctionT<Prism>, ShapeFunction, LibLagrangeP0 >
   Prism_Builder(LibLagrangeP0::library_namespace()+"."+Prism::type_name());

////////////////////////////////////////////////////////////////////////////////

void Prism::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  result[0] = 1.;
}

////////////////////////////////////////////////////////////////////////////////

void Prism::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  result(KSI,0) = 0.;
  result(ETA,0) = 0.;
  result(ZTA,0) = 0.;
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& Prism::local_coordinates()
{
  static const RealMatrix loc_coord =
      (RealMatrix(nb_nodes, dimensionality) <<

       1./3.,  1./3. , 0.

       ).finished();
  return loc_coord;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP0
} // mesh
} // cf3
