// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file Functions.hpp
/// @brief Functions describing Euler 2D physics
/// @author Willem Deconinck
/// @author Matteo Parsani


#ifndef cf3_physics_euler_euler2d_Functions_hpp
#define cf3_physics_euler_euler2d_Functions_hpp

#include "cf3/physics/euler/euler2d/Data.hpp"

namespace cf3 {
namespace physics {
namespace euler {
namespace euler2d {
  
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

/// @brief Linearize a left and right state using the Roe average
void compute_roe_average( const Data& left, const Data& right,
                          Data& roe );

/// @brief Rusanov Approximate Riemann solver
/// @note Very fast, but very dissipative
void compute_rusanov_flux( const Data& left, const Data& right, const ColVector_NDIM& normal,
                           RowVector_NEQS& flux, Real& wave_speed );

/// @brief Roe Approximate Riemann solver
/// @note Performs very well, but computationally expensive
void compute_roe_flux( const Data& left, const Data& right, const ColVector_NDIM& normal,
                       RowVector_NEQS& flux, Real& wave_speed );

/// @brief HLLE Approximate Riemann solver
/// @note Performs reasonably well, and reasonably performant
void compute_hlle_flux( const Data& left, const Data& right, const ColVector_NDIM& normal,
                        RowVector_NEQS& flux, Real& wave_speed );

/// @brief Compute the specific entropy from the primitive variables
void compute_specific_entropy( const Data& p, Real& specific_entropy );

/// @brief Calculate the Jacobian of the conserved variables with respect to the primitive variables
void compute_jacobian_conservative_wrt_primitive( const Data& p,
                                                  Matrix_NEQSxNEQS& dcons_dprim );

/// @brief Calculate the Jacobian of the primitive variables with respect to the conservative variables
void compute_jacobian_primitive_wrt_conservative( const Data& p,
                                                  Matrix_NEQSxNEQS& dprim_dcons );

//////////////////////////////////////////////////////////////////////////////////////////////

} // euler2d
} // euler
} // physics
} // cf3

#endif // cf3_physics_euler_euler2d_Functions_hpp
