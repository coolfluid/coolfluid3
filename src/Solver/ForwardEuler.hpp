// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_ForwardEuler_hpp
#define CF_Solver_ForwardEuler_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Solver/CIterativeSolver.hpp"

namespace CF {
namespace Solver {

////////////////////////////////////////////////////////////////////////////////

/// Solver component class
/// @author Tiago Quintino
/// @author Willem Deconinck
class Solver_API ForwardEuler : public Solver::CIterativeSolver {

public: // typedefs

  typedef boost::shared_ptr<ForwardEuler> Ptr;
  typedef boost::shared_ptr<ForwardEuler const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  ForwardEuler ( const CName& name );

  /// Virtual destructor
  virtual ~ForwardEuler();

  /// Get the class name
  static std::string type_name () { return "ForwardEuler"; }

  /// Configuration Options
  static void defineConfigProperties ( Common::PropertyList& options ) {}

  // functions specific to the ForwardEuler component
  
  virtual void do_stuff();

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

};

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_ForwardEuler_hpp
