// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_Core_ShockTube_hpp
#define CF_FVM_Core_ShockTube_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Solver/CWizard.hpp"
#include "FVM/Core/LibCore.hpp"

namespace CF {
namespace FVM {
namespace Core {

////////////////////////////////////////////////////////////////////////////////

/// Wizard to setup a 1D Euler shocktube simulation
/// @author Willem Deconinck
class FVM_Core_API ShockTube : public Solver::CWizard {

public: // typedefs

  typedef boost::shared_ptr<ShockTube> Ptr;
  typedef boost::shared_ptr<ShockTube const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  ShockTube ( const std::string& name );

  /// Virtual destructor
  virtual ~ShockTube();

  /// Get the class name
  static std::string type_name () { return "ShockTube"; }

  virtual std::string wizard_name() { return "shocktube_wizard"; }
  
  // functions specific to the ShockTube component

  /// @name SIGNALS
  //@{

  /// Signal to create a model
  void signal_create_model ( Common::SignalArgs& node );

  void signature_create_model( Common::SignalArgs& node);

  //@} END SIGNALS

};

////////////////////////////////////////////////////////////////////////////////

} // Core
} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_Core_ShockTube_hpp
