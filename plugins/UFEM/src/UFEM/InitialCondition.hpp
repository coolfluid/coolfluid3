// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_InitialCondition_hpp
#define cf3_UFEM_InitialCondition_hpp


#include "solver/Action.hpp"

#include "LibUFEM.hpp"

namespace cf3 {

namespace UFEM {

/// InitialCondition for UFEM problems
class UFEM_API InitialCondition : public solver::Action
{

public: // functions

  /// Contructor
  /// @param name of the component
  InitialCondition ( const std::string& name );

  virtual ~InitialCondition();

  /// Get the class name
  static std::string type_name () { return "InitialCondition"; }

  /// Execute the initial condition
  virtual void execute();

private:
  /// Triggered when the tag is changed
  void trigger_tag();
  /// List of option names associated with the initial conditions
  std::vector<std::string> m_variable_options;
};

} // UFEM
} // cf3


#endif // cf3_UFEM_InitialCondition_hpp
