// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "mesh/ShapeFunctionT.hpp"
#include "mesh/LagrangeP3/LibLagrangeP3.hpp"
#include "mesh/LagrangeP3/Line.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP3 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ShapeFunctionT<Line>, ShapeFunction, LibLagrangeP3 >
   Line_Builder(LibLagrangeP3::library_namespace()+"."+Line::type_name());

////////////////////////////////////////////////////////////////////////////////

void Line::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  const Real onesixteenth = 1.0/16.0;

  result[0] = -onesixteenth*(1.0 - mapped_coord[KSI])*(1.0 - 9.0*mapped_coord[KSI]*mapped_coord[KSI]);
  result[1] = -onesixteenth*(1.0 + mapped_coord[KSI])*(1.0 - 9.0*mapped_coord[KSI]*mapped_coord[KSI]);
  result[2] = -9.0*onesixteenth*(1.0 - mapped_coord[KSI]*mapped_coord[KSI])*(1.0 - 3.0*mapped_coord[KSI]);
  result[3] =  9.0*onesixteenth*(1.0 - mapped_coord[KSI]*mapped_coord[KSI])*(1.0 + 3.0*mapped_coord[KSI]);
}

////////////////////////////////////////////////////////////////////////////////

void Line::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  const Real onesixteenth = 1.0/16.0;

  result(KSI, 0) = -onesixteenth*( 27.0*mapped_coord[KSI]*mapped_coord[KSI] - 18.0*mapped_coord[KSI] - 1.0);
  result(KSI, 1) = -onesixteenth*(-27.0*mapped_coord[KSI]*mapped_coord[KSI] - 18.0*mapped_coord[KSI] + 1.0);
  result(KSI, 2) = 9.0*onesixteenth*( 9.0*mapped_coord[KSI]*mapped_coord[KSI] - 2.0*mapped_coord[KSI] - 3.0);
  result(KSI, 3) = 9.0*onesixteenth*(-9.0*mapped_coord[KSI]*mapped_coord[KSI] - 2.0*mapped_coord[KSI] + 3.0);
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& Line::local_coordinates()
{
  static const RealMatrix loc_coord =
      (RealMatrix(nb_nodes, dimensionality) <<

       -1.,
        1.,
       -1./3.,
        1./3.

       ).finished();
  return loc_coord;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP3
} // mesh
} // cf3
