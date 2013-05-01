// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "cf3/math/Defs.hpp"
#include "cf3/physics/euler/euler2d/Data.hpp"

namespace cf3 {
namespace physics {
namespace euler {
namespace euler2d {

////////////////////////////////////////////////////////////////////////////////////////////
  
void Data::compute_from_conservative(const RowVector_NEQS& _cons)
{
  // cons: rho, rho*u, rho*E
  cons = _cons;
  rho=cons[0];
  U[XX]=cons[1]/rho;
  U[YY]=cons[2]/rho;
  E=cons[3]/rho;
  U2=U[XX]*U[XX] + U[YY]*U[YY];
  p=(gamma-1.)*rho*(E - 0.5*U2);
  H=E+p/rho;
  c2=gamma*p/rho;
  c=std::sqrt(c2);
  M=std::sqrt(U2)/c;
  T=p/(rho*R);
}
    
void Data::compute_from_primitive(const RowVector_NEQS& prim)
{
  // prim: rho, u, p
  rho=prim[0];
  U[XX]=prim[1];
  U[YY]=prim[2];
  p=prim[3];
  U2=U[XX]*U[XX] + U[YY]*U[YY];
  c2=gamma*p/rho;
  c=std::sqrt(c2);
  H=c2/(gamma-1.)+0.5*U2;
  E=H-p/rho;
  M=std::sqrt(U2)/c;
  T=p/(rho*R);
  cons[0]=rho;
  cons[1]=rho*U[XX];
  cons[2]=rho*U[YY];
  cons[3]=rho*E;
}

//////////////////////////////////////////////////////////////////////////////////////////////

} // euler2d
} // euler
} // physics
} // cf3
