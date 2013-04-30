// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Data.hpp"

namespace cf3 {
namespace physics {
namespace euler {
namespace euler1d {

////////////////////////////////////////////////////////////////////////////////////////////
  
void Data::compute_from_conservative(const RowVector_NEQS& _cons)
{
  // cons: rho, rho*u, rho*E
  cons = _cons;
  rho=cons[0];
  u=cons[1]/rho;
  E=cons[2]/rho;
  u2=u*u;
  p=(gamma-1.)*rho*(E - 0.5*u2);
  H=E+p/rho;
  c2=gamma*p/rho;
  c=std::sqrt(c2);
  M=u/c;
  T=p/(rho*R);
}
    
void Data::compute_from_primitive(const RowVector_NEQS& prim)
{
  // prim: rho, u, p
  rho=prim[0];
  u=prim[1];
  p=prim[2];
  u2=u*u;
  c2=gamma*p/rho;
  c=std::sqrt(c2);
  H=c2/(gamma-1.)+0.5*u2;
  E=H-p/rho;
  M=u/c;
  T=p/(rho*R);
  cons[0]=rho;
  cons[1]=rho*u;
  cons[2]=rho*E;
}

//////////////////////////////////////////////////////////////////////////////////////////////

} // euler1D
} // euler
} // physics
} // cf3
