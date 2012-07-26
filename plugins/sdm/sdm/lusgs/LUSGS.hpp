// Copyright (C) 2010-2012 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file sdm/lusgs/LUSGS.hpp
/// @author Willem Deconinck, Matteo Parsani
///
/// This file includes the LUSGS component class.

#ifndef cf3_sdm_lusgs_LUSGS_hpp
#define cf3_sdm_lusgs_LUSGS_hpp

#include "Eigen/LU"
#include "math/MatrixTypes.hpp"
#include "sdm/IterativeSolver.hpp"
#include "sdm/lusgs/LibLUSGS.hpp"

namespace cf3 {
namespace sdm {
  class System;
namespace lusgs {

////////////////////////////////////////////////////////////////////////////////

/// @brief LUSGS iterative solver
///
/// @author Willem Deconinck, Matteo Parsani
class sdm_lusgs_API LUSGS : public IterativeSolver {

public: // functions

  /// Get the class name
  static std::string type_name () { return "LUSGS"; }

  /// Contructor
  /// @param name of the component
  LUSGS ( const std::string& name );

  /// Virtual destructor
  virtual ~LUSGS() {}

  /// execute the action
  virtual void execute ();

private: // functions

  /// @brief Compute the left-hand-side of the system,
  /// and store it in private variable m_lu
  void compute_system_lhs();

  /// @brief create the private variable m_system,
  /// according to the configuration option "system"
  void configure_system();

private: // data

  std::vector< std::vector< Eigen::FullPivLU<RealMatrix> > > m_lu;

  Handle<System> m_system;
};

////////////////////////////////////////////////////////////////////////////////

} // lusgs
} // sdm
} // cf3

#endif // cf3_sdm_lusgs_LUSGS_hpp
