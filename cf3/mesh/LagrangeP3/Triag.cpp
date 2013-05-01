// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "mesh/ShapeFunctionT.hpp"
#include "mesh/LagrangeP3/LibLagrangeP3.hpp"
#include "mesh/LagrangeP3/Triag.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP3 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ShapeFunctionT<Triag>, ShapeFunction, LibLagrangeP3 >
   Triag_Builder(LibLagrangeP3::library_namespace()+"."+Triag::type_name());

////////////////////////////////////////////////////////////////////////////////

void Triag::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  const Real L0 = 1. - mapped_coord[KSI] - mapped_coord[ETA];
  const Real L1 = mapped_coord[KSI];
  const Real L2 = mapped_coord[ETA];

  result[0] = 0.5*( 3.*L0 - 1. )*( 3.*L0 - 2.)*L0;
  result[1] = 0.5*( 3.*L1 - 1. )*( 3.*L1 - 2.)*L1;
  result[2] = 0.5*( 3.*L2 - 1. )*( 3.*L2 - 2.)*L2;

  result[3] = 9./2. * L0*L1*( 3.*L0 - 1. );
  result[4] = 9./2. * L0*L1*( 3.*L1 - 1. );

  result[5] = 9./2. * L1*L2*( 3.*L1 - 1. );
  result[6] = 9./2. * L1*L2*( 3.*L2 - 1. );

  result[7] = 9./2. * L2*L0*( 3.*L2 - 1. );
  result[8] = 9./2. * L2*L0*( 3.*L0 - 1. );

  result[9] = 27.*L0*L1*L2;
}

////////////////////////////////////////////////////////////////////////////////

void Triag::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  const Real L0 = 1.0 - mapped_coord[KSI] - mapped_coord[ETA];
  const Real L1 = mapped_coord[KSI];
  const Real L2 = mapped_coord[ETA];

  const Real dL0dxi  = -1.0;
  const Real dL0deta = -1.0;
  const Real dL1dxi  =  1.0;
  const Real dL1deta =  0.0;
  const Real dL2dxi  =  0.0;
  const Real dL2deta =  1.0;

  result(KSI,0) =  0.5 * dL0dxi  * ( 27.0*L0*L0 - 18.0*L0 + 2.0 );
  result(KSI,1) =  0.5 * dL1dxi  * ( 27.0*L1*L1 - 18.0*L1 + 2.0 );
  result(KSI,2) =  0.5 * dL2dxi  * ( 27.0*L2*L2 - 18.0*L2 + 2.0 );
  result(KSI,3) =  4.5 * ( dL0dxi*(6.0*L0*L1-L1) + dL1dxi*L0*(3.0*L0-1.0) );
  result(KSI,4) =  4.5 * ( dL1dxi*(6.0*L0*L1-L0) + dL0dxi*L1*(3.0*L1-1.0) );
  result(KSI,5) =  4.5 * ( dL1dxi*(6.0*L1*L2-L2) + dL2dxi*L1*(3.0*L1-1.0) );
  result(KSI,6) =  4.5 * ( dL2dxi*(6.0*L1*L2-L1) + dL1dxi*L2*(3.0*L2-1.0) );
  result(KSI,7) =  4.5 * ( dL2dxi*(6.0*L0*L2-L0) + dL0dxi*L2*(3.0*L2-1.0) );
  result(KSI,8) =  4.5 * ( dL0dxi*(6.0*L0*L2-L2) + dL2dxi*L0*(3.0*L0-1.0) );
  result(KSI,9) = 27.0 * ( dL0dxi*L1*L2 + L0*dL1dxi*L2 + L0*L1*dL2dxi );

  result(ETA,0) =  0.5 * dL0deta  * ( 27.0*L0*L0 - 18.0*L0 + 2.0 );
  result(ETA,1) =  0.5 * dL1deta  * ( 27.0*L1*L1 - 18.0*L1 + 2.0 );
  result(ETA,2) =  0.5 * dL2deta  * ( 27.0*L2*L2 - 18.0*L2 + 2.0 );
  result(ETA,3) =  4.5 * ( dL0deta*(6.0*L0*L1-L1) + dL1deta*L0*(3.0*L0-1.0) );
  result(ETA,4) =  4.5 * ( dL1deta*(6.0*L0*L1-L0) + dL0deta*L1*(3.0*L1-1.0) );
  result(ETA,5) =  4.5 * ( dL1deta*(6.0*L1*L2-L2) + dL2deta*L1*(3.0*L1-1.0) );
  result(ETA,6) =  4.5 * ( dL2deta*(6.0*L1*L2-L1) + dL1deta*L2*(3.0*L2-1.0) );
  result(ETA,7) =  4.5 * ( dL2deta*(6.0*L0*L2-L0) + dL0deta*L2*(3.0*L2-1.0) );
  result(ETA,8) =  4.5 * ( dL0deta*(6.0*L0*L2-L2) + dL2deta*L0*(3.0*L0-1.0) );
  result(ETA,9) = 27.0 * ( dL0deta*L1*L2 + L0*dL1deta*L2 + L0*L1*dL2deta );
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& Triag::local_coordinates()
{
  static const RealMatrix loc_coord =
      (RealMatrix(nb_nodes, dimensionality) <<

       0.,    0.,
       1.,    0.,
       0.,    1.,
       1./3., 0.,
       2./3., 0.,
       2./3., 1./3.,
       1./3., 2./3.,
       0.,    2./3.,
       0.,    1./3.,
       1./3., 1./3.

       ).finished();
  return loc_coord;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP3
} // mesh
} // cf3
