// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_BoundaryConditions_hpp
#define CF_RDM_BoundaryConditions_hpp

#include "Solver/ActionDirector.hpp"

#include "RDM/LibRDM.hpp"

namespace CF {
namespace RDM {

class BoundaryTerm;

/////////////////////////////////////////////////////////////////////////////////////

class RDM_API BoundaryConditions : public CF::Solver::ActionDirector {

public: // typedefs

  /// pointers
  typedef boost::shared_ptr<BoundaryConditions> Ptr;
  typedef boost::shared_ptr<BoundaryConditions const> ConstPtr;

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
                                                const std::vector<Common::URI>& regions );
  /// @name SIGNALS
  //@{

  /// creates a boundary BC
  void signal_create_boundary_condition( Common::SignalArgs& args );
  /// signature for @see signal_create_boundary_condition
  void signature_signal_create_boundary_condition( Common::SignalArgs& node );

  //@} END SIGNALS

private:

  Common::CActionDirector::Ptr m_weak_bcs;   ///< set of weak bcs

  Common::CActionDirector::Ptr m_strong_bcs; ///< set of strong bcs

};

/////////////////////////////////////////////////////////////////////////////////////


} // RDM
} // CF

#endif // CF_RDM_BoundaryConditions_hpp
