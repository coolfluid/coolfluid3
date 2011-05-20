// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_Steady_hpp
#define CF_RDM_Steady_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Solver/CWizard.hpp"

#include "RDM/Core/LibCore.hpp"

namespace CF {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////

/// Wizard to setup a scalar advection simulation
/// @author Tiago Quintino
class RDM_CORE_API Steady : public Solver::CWizard {

public: // typedefs

  typedef boost::shared_ptr<Steady> Ptr;
  typedef boost::shared_ptr<Steady const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  Steady ( const std::string& name );

  /// Virtual destructor
  virtual ~Steady();

  /// Get the class name
  static std::string type_name () { return "Steady"; }

  // functions specific to the Steady component

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

#endif // CF_RDM_Steady_hpp
