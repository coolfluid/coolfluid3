// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_InitialConditionConstant_hpp
#define cf3_UFEM_InitialConditionConstant_hpp


#include "solver/Action.hpp"

#include "LibUFEM.hpp"

namespace cf3 {

namespace UFEM {

/// InitialConditionConstant for UFEM problems, setting variables to a constant value
class UFEM_API InitialConditionConstant : public solver::Action
{

public: // functions

  /// Contructor
  /// @param name of the component
  InitialConditionConstant ( const std::string& name );

  virtual ~InitialConditionConstant();

  /// Get the class name
  static std::string type_name () { return "InitialConditionConstant"; }

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


#endif // cf3_UFEM_InitialConditionConstant_hpp
