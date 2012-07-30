// Copyright (C) 2010-2012 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file sdm/System.hpp
/// @author Willem Deconinck
///
/// This file includes the base class for Systems to be solved.

#ifndef cf3_sdm_System_hpp
#define cf3_sdm_System_hpp

#include "math/MatrixTypes.hpp"
#include "sdm/LibSDM.hpp"

namespace cf3 {

namespace mesh{ class Cells; }

namespace sdm {

/// @brief Base class for System definitions
///
/// Systems such as Backward Euler, BDF2, ESDIRK, ...
/// can be derived from this component. 
/// Inherited classes need to implement the functions
/// - compute_lhs()
/// - compute_rhs()
/// @author Willem Deconinck
class sdm_API System: public common::Component
{
public:

  /// @brief Type name
  // Unfortunately, a factory called System already exists in the math library, so give it different name!
  static std::string type_name() { return "ImplicitSystem"; }

  /// @brief Constructor
  System(const std::string& name) : common::Component(name) {}

  /// @brief Destructor
  virtual ~System() {}

  /// @brief Prepare the system before looping
  virtual void prepare() = 0;

  /// @brief loop cells
  virtual bool loop_cells(const Handle<const cf3::mesh::Cells> &cells) = 0;

  /// @brief compute the left-hand-side
  /// @param [in]   elem   Element index in cells that are looped over
  /// @param [out]  lhs    Computed left-hand-side of the system for this cell
  virtual void compute_lhs(const Uint elem, RealMatrix& lhs) = 0;

  /// @brief compute the right-hand-side
  /// @param [in]   elem   Element index in cells that are looped over
  /// @param [out]  rhs    Computed right-hand-side of the system for this cell
  virtual void compute_rhs(const Uint elem, RealVector& rhs) = 0;

  /// @brief Do updates with the solved unknowns
  ///
  /// System knows what to do with the solved unknowns
  /// @param [in]  elem       Element index in cells that are looped over
  /// @param [in]  unknowns   Solved unknowns by external solver
  /// @return convergence value
  virtual Real update(const Uint elem, const RealVector& unknowns) = 0;

  /// @brief Number of unknowns
  Uint nb_unknowns() const { return nb_cols(); }

  /// @brief Number of columns of the system left-hand-side matrix
  virtual Uint nb_cols() const = 0;

  /// @brief Number of rows of the system matrix
  virtual Uint nb_rows() const = 0;

};

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

#endif // cf3_sdm_System_hpp
