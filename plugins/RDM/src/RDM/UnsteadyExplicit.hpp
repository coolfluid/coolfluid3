// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_UnsteadyExplicit_hpp
#define CF_RDM_UnsteadyExplicit_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Solver/CWizard.hpp"

#include "RDM/LibRDM.hpp"

namespace CF {

 namespace Solver { class CModel; }

namespace RDM {

////////////////////////////////////////////////////////////////////////////////

/// Wizard to setup a scalar advection simulation
/// @author Tiago Quintino
class RDM_API UnsteadyExplicit : public Solver::CWizard {

public: // typedefs

  typedef boost::shared_ptr<UnsteadyExplicit> Ptr;
  typedef boost::shared_ptr<UnsteadyExplicit const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  UnsteadyExplicit ( const std::string& name );

  /// Virtual destructor
  virtual ~UnsteadyExplicit();

  /// Get the class name
  static std::string type_name () { return "UnsteadyExplicit"; }

  // functions specific to the UnsteadyExplicit component

  CF::Solver::CModel& create_model( const std::string& model_name,
                                    const std::string& physics_builder );

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

#endif // CF_RDM_UnsteadyExplicit_hpp
