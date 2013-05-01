// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_physics_euler_euler1D_Data_hpp
#define cf3_physics_euler_euler1D_Data_hpp

#include "cf3/physics/euler/euler1d/Types.hpp"

namespace cf3 {
namespace physics {
namespace euler {
namespace euler1d {

//////////////////////////////////////////////////////////////////////////////////////////////
  
struct Data
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW  ///< storing fixed-sized Eigen structures

  ColVector_NDIM coords;       ///< position in domain
  RowVector_NEQS cons;
    
  /// @name Gas constants
  //@{
  Real gamma;               ///< specific heat ratio
  Real R;                   ///< gas constant
  //@}

  Real rho;                 ///< density
  Real u;                   ///< velocity along XX
  Real u2;                  ///< velocity along XX squared
  Real H;                   ///< specific enthalpy
  Real c2;                  ///< square of speed of sound, very commonly used
  Real c;                   ///< speed of sound
  Real p;                   ///< pressure
  Real T;                   ///< temperature
  Real E;                   ///< specific total energy
  Real M;                   ///< Mach number
    
  /// @brief Compute the data given conservative state
  /// @pre gamma and R must have been set
  void compute_from_conservative(const RowVector_NEQS& cons);
  
  /// @brief Compute the data given primitive state
  /// @pre gamma and R must have been set
  void compute_from_primitive(const RowVector_NEQS& prim);
};

//////////////////////////////////////////////////////////////////////////////////////////////

} // euler1D
} // euler
} // physics
} // cf3

#endif // cf3_physics_euler_euler1D_Data_hpp
