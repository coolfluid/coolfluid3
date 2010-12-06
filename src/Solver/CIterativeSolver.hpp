// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CIterativeSolver_hpp
#define CF_Solver_CIterativeSolver_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Solver/CMethod.hpp"
#include "Solver/LibSolver.hpp"

namespace CF {
namespace Solver {

////////////////////////////////////////////////////////////////////////////////

/// Solver component class
/// Iterative solver component
/// @author Tiago Quintino
/// @author Willem Deconinck
class Solver_API CIterativeSolver : public Solver::CMethod {

public: // typedefs

  /// type of pointer to Component
  typedef boost::shared_ptr<CIterativeSolver> Ptr;
  /// type of pointer to constant Component
  typedef boost::shared_ptr<CIterativeSolver const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CIterativeSolver ( const std::string& name );

  /// Virtual destructor
  virtual ~CIterativeSolver();

  /// Get the class name
  static std::string type_name () { return "CIterativeSolver"; }

  // functions specific to the CIterativeSolver component

  virtual void solve() = 0;

protected: // data

  /// Maximum number of iterations
  Uint m_nb_iter;
  
};

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CIterativeSolver_hpp
