// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_ShockTube2D_hpp
#define CF_FVM_ShockTube2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Solver/CWizard.hpp"
#include "FVM/LibFVM.hpp"

namespace CF {
namespace FVM {

////////////////////////////////////////////////////////////////////////////////

/// Wizard to setup a 2D Euler shocktube simulation
/// @author Willem Deconinck
class FVM_API ShockTube2D : public Solver::CWizard {

public: // typedefs

  typedef boost::shared_ptr<ShockTube2D> Ptr;
  typedef boost::shared_ptr<ShockTube2D const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  ShockTube2D ( const std::string& name );

  /// Virtual destructor
  virtual ~ShockTube2D();

  /// Get the class name
  static std::string type_name () { return "ShockTube2D"; }

  virtual std::string wizard_name() { return "shocktube_wizard_2D"; }
  
  // functions specific to the ShockTube2D component

  /// @name SIGNALS
  //@{

  /// Signal to create a model
  void signal_create_model ( Common::SignalArgs& node );

  void signature_create_model( Common::SignalArgs& node);

  /// Signal to create a model
  void signal_setup_model ( Common::SignalArgs& node );

  void signature_setup_model( Common::SignalArgs& node);

  //@} END SIGNALS

};

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_ShockTube2D_hpp
