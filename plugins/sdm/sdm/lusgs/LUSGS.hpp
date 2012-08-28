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

#include "math/MatrixTypes.hpp"
#include "Eigen/LU"
#include "common/Action.hpp"
#include "sdm/lusgs/LibLUSGS.hpp"

namespace cf3 {
namespace common { class ActionDirector; }
namespace mesh { class Cells; class Dictionary; }
namespace sdm {
  class System;
namespace lusgs {

////////////////////////////////////////////////////////////////////////////////

/// @brief LU-SGS iterative solver
///
/// Non-linear Lower-Upper Symmetric Gauss Seidel iterative solver.
///
/// A non-linear system prescribed over the mesh is solved iteratively.
/// Computation of a cell's system right-hand-side makes use of
/// updated values of recently updated cell's solutions.
/// Alternating between forward and backward sweeps increases the convergence speed.
///
/// An example system to be configured is the implicit BackwardEuler system,
/// to advance the solution in time.
///
/// @author Willem Deconinck, Matteo Parsani
class sdm_lusgs_API LUSGS : public common::Action {

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

  Real forward_sweep();

  Real backward_sweep();

private: // data

  /// @brief Component describing how to compute the left- and
  /// right-hand-side of a system
  Handle<System> m_system;

  /// @brief Storage of LU-factorized system left-hand-side
  std::vector< std::vector< Eigen::FullPivLU<RealMatrix> > > m_lu;

  /// @brief Flag to alternate between forward and backward sweeps.
  enum SWEEP_DIR {FORWARD=1, BACKWARD=-1} m_sweep_direction;

  Handle<common::ActionDirector> m_pre_update;
  Handle<common::ActionDirector> m_post_update;
  Handle<mesh::Dictionary> m_dict;
};

////////////////////////////////////////////////////////////////////////////////

} // lusgs
} // sdm
} // cf3

#endif // cf3_sdm_lusgs_LUSGS_hpp
