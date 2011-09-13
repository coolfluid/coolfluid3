// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Mesh/ShapeFunctionT.hpp"
#include "Mesh/LagrangeP0/LibLagrangeP0.hpp"
#include "Mesh/LagrangeP0/Triag.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeP0 {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < ShapeFunctionT<Triag>, ShapeFunction, LibLagrangeP0 >
   Triag_Builder(LibLagrangeP0::library_namespace()+"."+Triag::type_name());

////////////////////////////////////////////////////////////////////////////////

void Triag::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  result[0] = 1.;
}

////////////////////////////////////////////////////////////////////////////////

void Triag::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  result(KSI,0) = 0.;
  result(ETA,0) = 0.;
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& Triag::local_coordinates()
{
  static const RealMatrix loc_coord =
      (RealMatrix(nb_nodes, dimensionality) <<

       1./3.,  1./3.

       ).finished();
  return loc_coord;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP0
} // Mesh
} // CF
