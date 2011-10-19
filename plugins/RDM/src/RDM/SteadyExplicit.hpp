// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_SteadyExplicit_hpp
#define cf3_RDM_SteadyExplicit_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Solver/CWizard.hpp"

#include "RDM/LibRDM.hpp"

namespace cf3 {

 namespace Solver { class CModel; }

namespace RDM {

////////////////////////////////////////////////////////////////////////////////

/// Wizard to setup a scalar advection simulation
/// @author Tiago Quintino
class RDM_API SteadyExplicit : public Solver::CWizard {

public: // typedefs

  typedef boost::shared_ptr<SteadyExplicit> Ptr;
  typedef boost::shared_ptr<SteadyExplicit const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  SteadyExplicit ( const std::string& name );

  /// Virtual destructor
  virtual ~SteadyExplicit();

  /// Get the class name
  static std::string type_name () { return "SteadyExplicit"; }

  // functions specific to the SteadyExplicit component

  cf3::Solver::CModel& create_model( const std::string& model_name,
                                    const std::string& physics_builder );

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

#endif // CF3_RDM_SteadyExplicit_hpp
