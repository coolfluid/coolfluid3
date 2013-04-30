// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_physics_lineuler_lineuler2d_Data_hpp
#define cf3_physics_lineuler_lineuler2d_Data_hpp

#include "cf3/physics/lineuler/lineuler2d/Types.hpp"

namespace cf3 {
namespace physics {
namespace lineuler {
namespace lineuler2d {

//////////////////////////////////////////////////////////////////////////////////////////////
  
struct Data
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW  ///< storing fixed-sized Eigen structures

  ColVector_NDIM coords;       ///< position in domain
  RowVector_NEQS cons;
    
  /// @name Gas constants
  //@{
  Real gamma;               ///< specific heat ratio
  //@}

  /// @name Mean flow
  //@{
  ColVector_NDIM U0;
  Real rho0;
  Real p0;
  Real c0;
  //@}
  
  Real rho;                 ///< density
  ColVector_NDIM U;         ///< velocity
  Real U2;                  ///< velocity squared
  Real p;                   ///< pressure
    
  /// @brief Compute the data given conservative state
  /// @pre gamma and R must have been set
  void compute_from_conservative(const RowVector_NEQS& cons);
  
  /// @brief Compute the data given primitive state
  /// @pre gamma and R must have been set
  void compute_from_primitive(const RowVector_NEQS& prim);
};

//////////////////////////////////////////////////////////////////////////////////////////////

} // lineuler2d
} // lineuler
} // physics
} // cf3

#endif // cf3_physics_lineuler_lineuler2d_Data_hpp
