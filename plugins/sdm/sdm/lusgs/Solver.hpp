// Copyright (C) 2010-2012 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file sdm/lusgs/Solver.hpp
/// @author Willem Deconinck, Matteo Parsani
///
/// This file includes the Solver component class.

#ifndef cf3_sdm_lusgs_Solver_hpp
#define cf3_sdm_lusgs_Solver_hpp

#include "sdm/Solver.hpp"
#include "sdm/lusgs/LUSGS.hpp"

namespace cf3 {
namespace sdm {
namespace lusgs {
	
	class LUSGS;

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
class sdm_lusgs_API Solver : public sdm::Solver {

public: // functions

  /// Get the class name
  static std::string type_name () { return "Solver"; }

  /// Contructor
  /// @param name of the component
  Solver ( const std::string& name );

  /// Virtual destructor
  virtual ~Solver() {}

  virtual void setup();

  virtual void step();

private: // functions

  /// @brief create the private variable m_system,
  /// according to the configuration option "system"
  void configure_lusgs();

private: // data

	Handle<LUSGS> m_lusgs;
};

////////////////////////////////////////////////////////////////////////////////

} // lusgs
} // sdm
} // cf3

#endif // cf3_sdm_lusgs_Solver_hpp
