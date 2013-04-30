// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_physics_navierstokes_navierstokes1d_Data_hpp
#define cf3_physics_navierstokes_navierstokes1d_Data_hpp

#include "cf3/physics/navierstokes/navierstokes1d/Types.hpp"
#include "cf3/physics/euler/euler1d/Data.hpp"

namespace cf3 {
namespace physics {
namespace navierstokes {
namespace navierstokes1d {

//////////////////////////////////////////////////////////////////////////////////////////////
  
struct Data : euler::euler1d::Data
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW  ///< storing fixed-sized Eigen structures
    
  /// @name Gas constants
  //@{
  Real mu;                  ///< dynamic viscosity
  Real k;                   ///< heat conductivity
  Real Cp;                  ///< Heat capacity
  //@}

  ColVector_NDIM grad_u;    ///< gradient of x velocity
  ColVector_NDIM grad_T;    ///< gradient of temperature
};

//////////////////////////////////////////////////////////////////////////////////////////////

} // navierstokes1d
} // navierstokes
} // physics
} // cf3

#endif // cf3_physics_navierstokes_navierstokes1d_Data_hpp
