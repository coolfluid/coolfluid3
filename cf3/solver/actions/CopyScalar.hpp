// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_CopyScalar_hpp
#define cf3_solver_actions_CopyScalar_hpp

#include "LibActions.hpp"

#include "Proto/ProtoAction.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

/// Copy a scalar field variable
class solver_actions_API CopyScalar : public Proto::ProtoAction
{
public: // functions
  /// Contructor
  /// @param name of the component
  CopyScalar ( const std::string& name );

  /// Virtual destructor
  virtual ~CopyScalar() {}

  /// Get the class name
  static std::string type_name () { return "CopyScalar"; }
  
private:
  void trigger_options();
};

/////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_actions_CopyScalar_hpp
