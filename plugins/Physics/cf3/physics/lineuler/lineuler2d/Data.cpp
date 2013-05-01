// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "cf3/math/Defs.hpp"
#include "cf3/physics/lineuler/lineuler2d/Data.hpp"

namespace cf3 {
namespace physics {
namespace lineuler {
namespace lineuler2d {

////////////////////////////////////////////////////////////////////////////////////////////
  
void Data::compute_from_conservative(const RowVector_NEQS& _cons)
{
  // cons: rho, rho0 U, p
  cons = _cons;
  rho=cons[0];
  U[XX]=cons[1]/rho0;
  U[YY]=cons[2]/rho0;
  p=cons[3];
  U2=U.norm();
}
    
void Data::compute_from_primitive(const RowVector_NEQS& prim)
{
  // prim: rho, U, p
  rho=prim[0];
  U[XX]=prim[1];
  U[YY]=prim[2];
  p=prim[3];
  U2=U.norm();
  cons[0]=rho;
  cons[1]=rho0*U[XX];
  cons[2]=rho0*U[YY];
  cons[3]=p;
}

//////////////////////////////////////////////////////////////////////////////////////////////

} // lineuler2d
} // lineuler
} // physics
} // cf3
