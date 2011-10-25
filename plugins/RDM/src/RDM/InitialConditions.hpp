// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_InitialConditions_hpp
#define cf3_RDM_InitialConditions_hpp

#include "solver/ActionDirector.hpp"

#include "RDM/LibRDM.hpp"

namespace cf3 {
namespace RDM {


/////////////////////////////////////////////////////////////////////////////////////

class RDM_API InitialConditions : public cf3::solver::ActionDirector {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr<InitialConditions> Ptr;
  typedef boost::shared_ptr<InitialConditions const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  InitialConditions ( const std::string& name );

  /// Virtual destructor
  virtual ~InitialConditions() {}

  /// Get the class name
  static std::string type_name () { return "InitialConditions"; }

  /// execute the action
  virtual void execute ();

  /// @name SIGNALS
  //@{

  /// adds an initialization
  void signal_create_initial_condition( common::SignalArgs& args );
  /// signature for @see signal_create_initial_condition
  void signature_signal_create_initial_condition( common::SignalArgs& node );

  //@} END SIGNALS

};

/////////////////////////////////////////////////////////////////////////////////////


} // RDM
} // cf3

#endif // cf3_RDM_InitialConditions_hpp
