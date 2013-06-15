// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "mesh/ShapeFunctionT.hpp"
#include "mesh/LagrangeP1/LibLagrangeP1.hpp"
#include "mesh/LagrangeP1/Hexa.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ShapeFunctionT<Hexa>, ShapeFunction, LibLagrangeP1 >
   Hexa_Builder(LibLagrangeP1::library_namespace()+"."+Hexa::type_name());

////////////////////////////////////////////////////////////////////////////////

void Hexa::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  const Real xi   = mapped_coord[KSI];
  const Real eta  = mapped_coord[ETA];
  const Real zeta = mapped_coord[ZTA];

  const Real a1 = (1 + xi);
  const Real a2 = (1 - xi);

  const Real b1 = (1 + eta);
  const Real b2 = (1 - eta);

  const Real c1 = (1 + zeta);
  const Real c2 = (1 - zeta);

  result[0] = a2*b2*c2;
  result[1] = a1*b2*c2;
  result[2] = a1*b1*c2;
  result[3] = a2*b1*c2;
  result[4] = a2*b2*c1;
  result[5] = a1*b2*c1;
  result[6] = a1*b1*c1;
  result[7] = a2*b1*c1;

  result *= 0.125;
}

////////////////////////////////////////////////////////////////////////////////

void Hexa::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  const Real xi   = mapped_coord[KSI];
  const Real eta  = mapped_coord[ETA];
  const Real zeta = mapped_coord[ZTA];

  const Real a1 = (1 + xi);
  const Real a2 = (1 - xi);

  const Real b1 = (1 + eta);
  const Real b2 = (1 - eta);

  const Real c1 = (1 + zeta);
  const Real c2 = (1 - zeta);

  result(KSI, 0) = -0.125 * b2*c2;
  result(ETA, 0) = -0.125 * a2*c2;
  result(ZTA, 0) = -0.125 * a2*b2;

  result(KSI, 1) =  0.125 * b2*c2;
  result(ETA, 1) = -0.125 * a1*c2;
  result(ZTA, 1) = -0.125 * a1*b2;

  result(KSI, 2) =  0.125 * b1*c2;
  result(ETA, 2) =  0.125 * a1*c2;
  result(ZTA, 2) = -0.125 * a1*b1;

  result(KSI, 3) = -0.125 * b1*c2;
  result(ETA, 3) =  0.125 * a2*c2;
  result(ZTA, 3) = -0.125 * a2*b1;

  result(KSI, 4) = -0.125 * b2*c1;
  result(ETA, 4) = -0.125 * a2*c1;
  result(ZTA, 4) =  0.125 * a2*b2;

  result(KSI, 5) =  0.125 * b2*c1;
  result(ETA, 5) = -0.125 * a1*c1;
  result(ZTA, 5) =  0.125 * a1*b2;

  result(KSI, 6) =  0.125 * b1*c1;
  result(ETA, 6) =  0.125 * a1*c1;
  result(ZTA, 6) =  0.125 * a1*b1;

  result(KSI, 7) = -0.125 * b1*c1;
  result(ETA, 7) =  0.125 * a2*c1;
  result(ZTA, 7) =  0.125 * a2*b1;
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& Hexa::local_coordinates()
{
  static const RealMatrix loc_coord =
      (RealMatrix(nb_nodes, dimensionality) <<

       -1., -1., -1.,  // node 0
        1., -1., -1.,  // node 1
        1.,  1., -1.,  // node 2
       -1.,  1., -1.,  // node 3
       -1., -1.,  1.,  // node 4
        1., -1.,  1.,  // node 5
        1.,  1.,  1.,  // node 6
       -1.,  1.,  1.   // node 7

       ).finished();
  return loc_coord;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // mesh
} // cf3
