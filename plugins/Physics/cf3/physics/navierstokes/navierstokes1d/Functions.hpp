// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file Functions.hpp
/// @brief Functions describing navierstokes 1D physics
/// @author Willem Deconinck

#ifndef cf3_physics_navierstokes_navierstokes1d_Functions_hpp
#define cf3_physics_navierstokes_navierstokes1d_Functions_hpp

#include "cf3/physics/navierstokes/navierstokes1d/Data.hpp"

namespace cf3 {
namespace physics {
namespace navierstokes {
namespace navierstokes1d {
  
//////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Diffusive flux in conservative form
void compute_diffusive_flux( const Data& p, const ColVector_NDIM& normal,
                             RowVector_NEQS& flux );

/// @brief Diffusive flux in conservative form
void compute_diffusive_flux( const Data& p, const ColVector_NDIM& normal,
                             RowVector_NEQS& flux, Real& wave_speed );

/// @brief Maximum absolute wave speed
void compute_diffusive_wave_speed( const Data& p, const ColVector_NDIM& normal,
                                   Real& wave_speed );

//////////////////////////////////////////////////////////////////////////////////////////////

} // navierstokes1d
} // navierstokes
} // physics
} // cf3

#endif // cf3_physics_navierstokes_navierstokes1d_Functions_hpp
