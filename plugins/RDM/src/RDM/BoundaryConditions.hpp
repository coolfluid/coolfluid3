// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_BoundaryConditions_hpp
#define cf3_RDM_BoundaryConditions_hpp

#include "solver/ActionDirector.hpp"

#include "RDM/LibRDM.hpp"

namespace cf3 {
namespace RDM {

class BoundaryTerm;

/////////////////////////////////////////////////////////////////////////////////////

class RDM_API BoundaryConditions : public cf3::solver::ActionDirector {

public: // typedefs

  /// pointers
  
  

public: // functions
  /// Contructor
  /// @param name of the component
  BoundaryConditions ( const std::string& name );

  /// Virtual destructor
  virtual ~BoundaryConditions() {}

  /// Get the class name
  static std::string type_name () { return "BoundaryConditions"; }

  /// execute the action
  virtual void execute ();

  RDM::BoundaryTerm& create_boundary_condition( const std::string& type,
                                                const std::string& name,
                                                const std::vector<common::URI>& regions );
  /// @name SIGNALS
  //@{

  /// creates a boundary BC
  void signal_create_boundary_condition( common::SignalArgs& args );
  /// signature for @see signal_create_boundary_condition
  void signature_signal_create_boundary_condition( common::SignalArgs& node );

  //@} END SIGNALS

private:

  Handle< common::ActionDirector > m_weak_bcs;   ///< set of weak bcs

  Handle< common::ActionDirector > m_strong_bcs; ///< set of strong bcs

};

/////////////////////////////////////////////////////////////////////////////////////


} // RDM
} // cf3

#endif // cf3_RDM_BoundaryConditions_hpp
