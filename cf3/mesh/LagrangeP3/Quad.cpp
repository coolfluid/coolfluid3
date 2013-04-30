// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "mesh/ShapeFunctionT.hpp"
#include "mesh/LagrangeP3/LibLagrangeP3.hpp"
#include "mesh/LagrangeP3/Quad.hpp"

namespace cf3 {
namespace mesh {
namespace LagrangeP3 {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ShapeFunctionT<Quad>, ShapeFunction, LibLagrangeP3 >
   Quad_Builder(LibLagrangeP3::library_namespace()+"."+Quad::type_name());

////////////////////////////////////////////////////////////////////////////////

void Quad::compute_value(const MappedCoordsT& mapped_coord, ValueT& result)
{
  const Real ksi = mapped_coord[KSI];
  const Real eta = mapped_coord[ETA];

  const Real onethird = 1.0/3.0;

  const Real L0ksi  = - 9.0/16.0 * (ksi+onethird) * (ksi-onethird) * (ksi-1.0);
  const Real L1ksi  =  27.0/16.0 *    (ksi+1.0)   * (ksi-onethird) * (ksi-1.0);
  const Real L2ksi  = -27.0/16.0 *    (ksi+1.0)   * (ksi+onethird) * (ksi-1.0);
  const Real L3ksi  =   9.0/16.0 *    (ksi+1.0)   * (ksi+onethird) * (ksi-onethird);

  const Real L0eta = - 9.0/16.0 * (eta+onethird) * (eta-onethird) * (eta-1.0);
  const Real L1eta =  27.0/16.0 *    (eta+1.0)   * (eta-onethird) * (eta-1.0);
  const Real L2eta = -27.0/16.0 *    (eta+1.0)   * (eta+onethird) * (eta-1.0);
  const Real L3eta =   9.0/16.0 *    (eta+1.0)   * (eta+onethird) * (eta-onethird);

  result[0]  = L0ksi * L0eta;
  result[1]  = L3ksi * L0eta;
  result[2]  = L3ksi * L3eta;
  result[3]  = L0ksi * L3eta;
  result[4]  = L1ksi * L0eta;
  result[5]  = L2ksi * L0eta;
  result[6]  = L3ksi * L1eta;
  result[7]  = L3ksi * L2eta;
  result[8]  = L2ksi * L3eta;
  result[9]  = L1ksi * L3eta;
  result[10] = L0ksi * L2eta;
  result[11] = L0ksi * L1eta;
  result[12] = L1ksi * L1eta;
  result[13] = L2ksi * L1eta;
  result[14] = L2ksi * L2eta;
  result[15] = L1ksi * L2eta;
}

////////////////////////////////////////////////////////////////////////////////

void Quad::compute_gradient(const MappedCoordsT& mapped_coord, GradientT& result)
{
  const Real ksi = mapped_coord[KSI];
  const Real eta = mapped_coord[ETA];

  const Real onethird = 1.0/3.0;

  const Real L0ksi  = - 9.0/16.0 * (ksi+onethird) * (ksi-onethird) * (ksi-1.0);
  const Real L1ksi  =  27.0/16.0 *    (ksi+1.0)   * (ksi-onethird) * (ksi-1.0);
  const Real L2ksi  = -27.0/16.0 *    (ksi+1.0)   * (ksi+onethird) * (ksi-1.0);
  const Real L3ksi  =   9.0/16.0 *    (ksi+1.0)   * (ksi+onethird) * (ksi-onethird);

  const Real L0eta = - 9.0/16.0 * (eta+onethird) * (eta-onethird) * (eta-1.0);
  const Real L1eta =  27.0/16.0 *    (eta+1.0)   * (eta-onethird) * (eta-1.0);
  const Real L2eta = -27.0/16.0 *    (eta+1.0)   * (eta+onethird) * (eta-1.0);
  const Real L3eta =   9.0/16.0 *    (eta+1.0)   * (eta+onethird) * (eta-onethird);

  const Real dL0dksi = - 9.0/16.0 * (   (ksi-onethird)*(ksi-1.0)    + (ksi+onethird)*(ksi-1.0) + (ksi+onethird)*(ksi-onethird) );
  const Real dL1dksi =  27.0/16.0 * (   (ksi-onethird)*(ksi-1.0)    +    (ksi+1.0)*(ksi-1.0)   +   (ksi+1.0)*(ksi-onethird) );
  const Real dL2dksi = -27.0/16.0 * (   (ksi+onethird)*(ksi-1.0)    +    (ksi+1.0)*(ksi-1.0)   +   (ksi+1.0)*(ksi+onethird) );
  const Real dL3dksi =   9.0/16.0 * ( (ksi+onethird)*(ksi-onethird) + (ksi+1.0)*(ksi-onethird) +   (ksi+1.0)*(ksi+onethird) );

  const Real dL0deta = - 9.0/16.0 * (   (eta-onethird)*(eta-1.0)    + (eta+onethird)*(eta-1.0) + (eta+onethird)*(eta-onethird) );
  const Real dL1deta =  27.0/16.0 * (   (eta-onethird)*(eta-1.0)    +    (eta+1.0)*(eta-1.0)   +   (eta+1.0)*(eta-onethird) );
  const Real dL2deta = -27.0/16.0 * (   (eta+onethird)*(eta-1.0)    +    (eta+1.0)*(eta-1.0)   +   (eta+1.0)*(eta+onethird) );
  const Real dL3deta =   9.0/16.0 * ( (eta+onethird)*(eta-onethird) + (eta+1.0)*(eta-onethird) +   (eta+1.0)*(eta+onethird) );

  result(KSI,0)  = dL0dksi * L0eta;
  result(KSI,1)  = dL3dksi * L0eta;
  result(KSI,2)  = dL3dksi * L3eta;
  result(KSI,3)  = dL0dksi * L3eta;
  result(KSI,4)  = dL1dksi * L0eta;
  result(KSI,5)  = dL2dksi * L0eta;
  result(KSI,6)  = dL3dksi * L1eta;
  result(KSI,7)  = dL3dksi * L2eta;
  result(KSI,8)  = dL2dksi * L3eta;
  result(KSI,9)  = dL1dksi * L3eta;
  result(KSI,10) = dL0dksi * L2eta;
  result(KSI,11) = dL0dksi * L1eta;
  result(KSI,12) = dL1dksi * L1eta;
  result(KSI,13) = dL2dksi * L1eta;
  result(KSI,14) = dL2dksi * L2eta;
  result(KSI,15) = dL1dksi * L2eta;

  result(ETA,0)  = L0ksi * dL0deta;
  result(ETA,1)  = L3ksi * dL0deta;
  result(ETA,2)  = L3ksi * dL3deta;
  result(ETA,3)  = L0ksi * dL3deta;
  result(ETA,4)  = L1ksi * dL0deta;
  result(ETA,5)  = L2ksi * dL0deta;
  result(ETA,6)  = L3ksi * dL1deta;
  result(ETA,7)  = L3ksi * dL2deta;
  result(ETA,8)  = L2ksi * dL3deta;
  result(ETA,9)  = L1ksi * dL3deta;
  result(ETA,10) = L0ksi * dL2deta;
  result(ETA,11) = L0ksi * dL1deta;
  result(ETA,12) = L1ksi * dL1deta;
  result(ETA,13) = L2ksi * dL1deta;
  result(ETA,14) = L2ksi * dL2deta;
  result(ETA,15) = L1ksi * dL2deta;
}

////////////////////////////////////////////////////////////////////////////////

const RealMatrix& Quad::local_coordinates()
{
  static const RealMatrix loc_coord =
      (RealMatrix(nb_nodes, dimensionality) <<

       -1.,     -1.,
        1.,     -1.,
        1.,      1.,
       -1.,      1.,
       -1./3.,  -1.,
        1./3.,  -1.,
        1.,     -1./3.,
        1.,      1./3.,
        1./3.,   1.,
       -1./3.,   1.,
       -1.,      1./3.,
       -1.,     -1./3.,
       -1./3.,  -1./3.,
        1./3.,  -1./3.,
        1./3.,   1./3.,
       -1./3.,   1./3.

       ).finished();
  return loc_coord;
}

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP3
} // mesh
} // cf3
