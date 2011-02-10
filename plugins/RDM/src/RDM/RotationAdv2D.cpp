// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "RDM/RotationAdv2D.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

RotationAdv2D::RotationAdv2D()
{
}

/////////////////////////////////////////////////////////////////////////////////////

RotationAdv2D::~RotationAdv2D()
{
}

/////////////////////////////////////////////////////////////////////////////////////

/// Function to compute the variable beta
Real RotationAdv2D::beta(const RealVector2 & coord, const Real sf, const RealVector2 & grad_sf)
{
   return coord[YY] * grad_sf[XX] - coord[XX] * grad_sf[YY];
}

/////////////////////////////////////////////////////////////////////////////////////

/// Compute the operator applied to solution
Real RotationAdv2D::residual(const RealVector2 & coord, const Real sol, const RealVector2 & grad_sol)
{
   return coord[YY] * grad_sol[XX] - coord[XX] * grad_sol[YY];
}

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////
