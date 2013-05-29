// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file Functions.hpp
/// @brief Functions describing Euler 2d physics
/// @author Willem Deconinck

#ifndef cf3_physics_lineuler_lineuler2d_Functions_hpp
#define cf3_physics_lineuler_lineuler2d_Functions_hpp

#include "cf3/physics/lineuler/lineuler2d/Data.hpp"

namespace cf3 {
namespace physics {
namespace lineuler {
namespace lineuler2d {
  
//////////////////////////////////////////////////////////////////////////////////////////////

/// @brief Convective flux in conservative form
void compute_convective_flux( const Data& p, const ColVector_NDIM& normal,
                              RowVector_NEQS& flux );

/// @brief Convective flux in conservative form, and maximum absolute wave speed
void compute_convective_flux( const Data& p, const ColVector_NDIM& normal,
                              RowVector_NEQS& flux, Real& wave_speed );

/// @brief Maximum absolute wave speed
void compute_convective_wave_speed( const Data& p, const ColVector_NDIM& normal,
                                    Real& wave_speed );

/// @brief Eigenvalues or wave speeds projected on a given normal
void compute_convective_eigenvalues( const Data& p, const ColVector_NDIM& normal,
                                     RowVector_NEQS& eigen_values );

/// @brief Right eigenvectors projected on a given normal
void compute_convective_right_eigenvectors( const Data& p, const ColVector_NDIM& normal,
                                            Matrix_NEQSxNEQS& right_eigenvectors );

/// @brief Left eigenvectors projected on a given normal
void compute_convective_left_eigenvectors( const Data& p, const ColVector_NDIM& normal,
                                           Matrix_NEQSxNEQS& left_eigenvectors );

/// @brief Absolute flux jacobian projected on a given normal
void compute_absolute_flux_jacobian( const Data& p, const ColVector_NDIM& normal,
                                     Matrix_NEQSxNEQS& absolute_flux_jacobian);

void char_to_cons( const RowVector_NEQS& characteristic,
                   const ColVector_NDIM& characteristic_normal,
                   const Real& c0,
                   RowVector_NEQS& conservative );

void cons_to_char( const RowVector_NEQS& conservative,
                   const ColVector_NDIM& characteristic_normal,
                   const Real& c0,
                   RowVector_NEQS& characteristic );

// ------- Riemann Solvers ---------

/// @brief Rusanov Approximate Riemann solver
/// @note Very fast, but very dissipative
/// @note Bad choice for Linearized Euler, since the exact CIR scheme is cheap too.
void compute_rusanov_flux( const Data& left, const Data& right, const ColVector_NDIM& normal,
                           RowVector_NEQS& flux, Real& wave_speed );

/// @brief CIR (Courant Isaacson Rees) Flux Splitting Upwind scheme for linear hyperbolic system
/// Riemann solver
/// F = 0.5 ( FL + FR ) - 0.5 |A| ( UR - UL )
/// @note Use this to solve the Riemann problem
void compute_cir_flux( const Data& left, const Data& right, const ColVector_NDIM& normal,
                       RowVector_NEQS& flux, Real& wave_speed );

//////////////////////////////////////////////////////////////////////////////////////////////

} // lineuler2d
} // lineuler
} // physics
} // cf3

#endif // cf3_physics_lineuler_lineuler2d_Functions_hpp
