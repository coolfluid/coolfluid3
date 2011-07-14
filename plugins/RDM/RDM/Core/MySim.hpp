// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_MySim_hpp
#define CF_RDM_MySim_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Solver/CWizard.hpp"

#include "RDM/Core/LibCore.hpp"

namespace CF {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////

/// Wizard to setup a scalar advection simulation
/// @author Tiago Quintino
class RDM_Core_API MySim : public Solver::CWizard {

public: // typedefs

  typedef boost::shared_ptr<MySim> Ptr;
  typedef boost::shared_ptr<MySim const> ConstPtr;

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
  void signal_create_model ( Common::SignalArgs& node );

  void signature_create_model( Common::SignalArgs& node);

  //@} END SIGNALS

};

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_MySim_hpp
