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

  typedef boost::shared_ptr<CIterativeSolver> Ptr;
  typedef boost::shared_ptr<CIterativeSolver const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CIterativeSolver ( const CName& name );

  /// Virtual destructor
  virtual ~CIterativeSolver();

  /// Get the class name
  static std::string type_name () { return "CIterativeSolver"; }

  /// Configuration Options
  static void defineConfigProperties ( Common::PropertyList& options ) {}

  // functions specific to the CIterativeSolver component
  
private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

};

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CIterativeSolver_hpp
