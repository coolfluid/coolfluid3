// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_ZeroLSS_hpp
#define cf3_solver_ZeroLSS_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Action.hpp"

#include "LibLSS.hpp"

namespace cf3 {
namespace math {
namespace LSS {

class System;

////////////////////////////////////////////////////////////////////////////////

/// ZeroLSS wraps a linear system solver in an action that will execute the solve
/// @author Bart Janssens
class LSS_API ZeroLSS : public common::Action
{
public: // functions

  /// Contructor
  /// @param name of the component
  ZeroLSS ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "ZeroLSS"; }

  /// Run the underlying linear system solver
  void execute();

private:
  Handle<System> m_lss;
};

////////////////////////////////////////////////////////////////////////////////

} // LSS
} // math
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_ZeroLSS_hpp
