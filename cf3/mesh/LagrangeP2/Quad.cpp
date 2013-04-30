// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "mesh/ShapeFunctionT.hpp"
#include "mesh/LagrangeP2/LibLagrangeP2.hpp"
#include "mesh/LagrangeP2/Quad.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP2 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ShapeFunctionT<Quad>, ShapeFunction, LibLagrangeP2 >
   Quad_Builder(LibLagrangeP2::library_namespace()+"."+Quad::type_name());

////////////////////////////////////////////////////////////////////////////////

void Quad::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  const Real ksi = mapped_coord[KSI];
  const Real eta = mapped_coord[ETA];

  const Real ksi2 = ksi*ksi;
  const Real eta2 = eta*eta;
  const Real ksi_eta = ksi*eta;

  result[0] =  0.25 * (1.0 - ksi)  * (1.0 - eta)  * ksi_eta;
  result[1] = -0.25 * (1.0 + ksi)  * (1.0 - eta)  * ksi_eta;
  result[2] =  0.25 * (1.0 + ksi)  * (1.0 + eta)  * ksi_eta;
  result[3] = -0.25 * (1.0 - ksi)  * (1.0 + eta)  * ksi_eta;
  result[4] = -0.5  * (1.0 - ksi2) * (1.0 - eta)  * eta;
  result[5] =  0.5  * (1.0 + ksi)  * (1.0 - eta2) * ksi;
  result[6] =  0.5  * (1.0 - ksi2) * (1.0 + eta)  * eta;
  result[7] = -0.5  * (1.0 - ksi)  * (1.0 - eta2) * ksi;
  result[8] =         (1.0 - ksi2) * (1.0 - eta2);
}

////////////////////////////////////////////////////////////////////////////////

void Quad::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  const Real ksi = mapped_coord[KSI];
  const Real eta = mapped_coord[ETA];
  const Real ksi2 = ksi*ksi;
  const Real eta2 = eta*eta;
  const Real ksi_eta = ksi*eta;
  const Real ksi2_eta = ksi2*eta;
  const Real ksi_eta2 = ksi*eta2;

  result(KSI,0) =  0.25 * (eta - 2.*ksi_eta - eta2 + 2.*ksi_eta2);
  result(KSI,1) = -0.25 * (eta + 2.*ksi_eta - eta2 - 2.*ksi_eta2);
  result(KSI,2) =  0.25 * (eta + 2.*ksi_eta + eta2 + 2.*ksi_eta2);
  result(KSI,3) = -0.25 * (eta - 2.*ksi_eta + eta2 - 2.*ksi_eta2);
  result(KSI,4) = -0.5  * (-2.*ksi_eta + 2.*ksi_eta2);
  result(KSI,5) =  0.5  * (1. - eta2 + 2.*ksi - 2.*ksi_eta2);
  result(KSI,6) =  0.5  * (-2.*ksi_eta - 2.*ksi_eta2);
  result(KSI,7) = -0.5  * (1. - eta2 - 2.*ksi + 2.*ksi_eta2);
  result(KSI,8) =  2.*ksi_eta2 - 2.*ksi;

  result(ETA,0) =  0.25 * (ksi - ksi2 - 2.*ksi_eta + 2.*ksi2_eta);
  result(ETA,1) = -0.25 * (ksi + ksi2 - 2.*ksi_eta - 2.*ksi2_eta);
  result(ETA,2) =  0.25 * (ksi + ksi2 + 2.*ksi_eta + 2.*ksi2_eta);
  result(ETA,3) = -0.25 * (ksi - ksi2 + 2.*ksi_eta - 2.*ksi2_eta);
  result(ETA,4) = -0.5 * (1. - ksi2 - 2.*eta + 2.*ksi2_eta);
  result(ETA,5) =  0.5 * (-2.*ksi_eta - 2.*ksi2_eta);
  result(ETA,6) =  0.5 * (1. - ksi2 + 2.*eta - 2.*ksi2_eta);
  result(ETA,7) = -0.5 * (-2.*ksi_eta + 2.*ksi2_eta);
  result(ETA,8) =  2.*ksi2_eta - 2.*eta;
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& Quad::local_coordinates()
{
  static const RealMatrix loc_coord =
      (RealMatrix(nb_nodes, dimensionality) <<

       -1., -1.,
        1., -1.,
        1.,  1.,
       -1.,  1.,
        0., -1.,
        1.,  0.,
        0.,  1.,
       -1.,  0.,
        0.,  0.

       ).finished();
  return loc_coord;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP2
} // mesh
} // cf3
