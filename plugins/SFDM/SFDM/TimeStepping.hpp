// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_TimeStepping_hpp
#define cf3_SFDM_TimeStepping_hpp

#include "Solver/ActionDirector.hpp"

#include "Solver/CTime.hpp"
#include "SFDM/LibSFDM.hpp"

namespace cf3 {

  namespace Solver { class CTime; }

namespace SFDM {


/////////////////////////////////////////////////////////////////////////////////////

class SFDM_API TimeStepping : public cf3::Solver::ActionDirector {

public: // typedefs

  typedef boost::shared_ptr<TimeStepping> Ptr;
  typedef boost::shared_ptr<TimeStepping const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  TimeStepping ( const std::string& name );

  /// Virtual destructor
  virtual ~TimeStepping() {}

  /// Get the class name
  static std::string type_name () { return "TimeStepping"; }

  /// execute the action
  virtual void execute ();

  common::CActionDirector& pre_actions()  { return *m_pre_actions; }
  common::CActionDirector& post_actions() { return *m_post_actions; }

  cf3::Solver::CTime&       time()         { return *m_time; }

  /// @name SIGNALS
  //@{

  //@} END SIGNALS

private: // functions

  /// @returns true if any of the stop criteria is achieved
  bool stop_condition();
  /// raises event when timestep is done
  void raise_timestep_done();

private: // data

  boost::shared_ptr< Solver::CTime > m_time;   ///< component tracking time

  common::CActionDirector::Ptr m_pre_actions;  ///< set of actions before non-linear solve

  common::CActionDirector::Ptr m_post_actions; ///< set of actions after non-linear solve

};

/////////////////////////////////////////////////////////////////////////////////////


} // SFDM
} // cf3

#endif // cf3_SFDM_TimeStepping_hpp
