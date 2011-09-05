// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Mesh/ShapeFunctionT.hpp"
#include "Mesh/LagrangeP1/Triag.hpp"

namespace CF {
namespace Mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < ShapeFunctionT<Triag>, ShapeFunction, LibLagrangeP1 >
   Triag_Builder(LibLagrangeP1::library_namespace()+"."+Triag::type_name());

////////////////////////////////////////////////////////////////////////////////

Triag::ValueT Triag::value(const MappedCoordsT& mapped_coord)
{
  ValueT result;
  compute_value(mapped_coord,result);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

Triag::GradientT Triag::gradient(const MappedCoordsT& mapped_coord)
{
  GradientT result;
  compute_gradient(mapped_coord,result);
  return result;
}

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

RealMatrix Triag::m_local_coordinates =  ( RealMatrix(Triag::nb_nodes, Triag::dimensionality) <<
    0.,  0.,
    1.,  0.,
    0.,  1.
).finished();

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // Mesh
} // CF
