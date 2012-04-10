// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_InitialConditions_hpp
#define cf3_UFEM_InitialConditions_hpp

#include "solver/ActionDirector.hpp"
#include "common/OptionURI.hpp"

#include "solver/Solver.hpp"

#include "LibUFEM.hpp"
#include "InitialCondition.hpp"

namespace cf3 {

namespace UFEM {

/// InitialConditions for UFEM problems
class UFEM_API InitialConditions : public solver::ActionDirector
{

public: // functions

  /// Contructor
  /// @param name of the component
  InitialConditions ( const std::string& name );

  virtual ~InitialConditions();

  /// Get the class name
  static std::string type_name () { return "InitialConditions"; }

  /// Create an initial condition for the field the given tag
  Handle<InitialCondition> create_initial_condition(const std::string& tag);
  
  void signal_create_initial_condition(common::SignalArgs& args);
  void signature_create_initial_condition(common::SignalArgs& args);
};

} // UFEM
} // cf3


#endif // cf3_UFEM_InitialConditions_hpp
