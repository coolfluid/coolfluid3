// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_MySim_hpp
#define cf3_RDM_MySim_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/CWizard.hpp"

#include "RDM/LibRDM.hpp"

namespace cf3 {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////

/// Wizard to setup a scalar advection simulation
/// @author Tiago Quintino
class RDM_API MySim : public solver::CWizard {

public: // typedefs

  
  

public: // functions

  /// Contructor
  /// @param name of the component
  MySim ( const std::string& name );

  /// Virtual destructor
  virtual ~MySim();

  /// Get the class name
  static std::string type_name () { return "MySim"; }

  // functions specific to the MySim component

  /// @name SIGNALS
  //@{

  /// Signal to create a model
  void signal_create_model ( common::SignalArgs& node );

  void signature_create_model( common::SignalArgs& node);

  //@} END SIGNALS

};

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_RDM_MySim_hpp
