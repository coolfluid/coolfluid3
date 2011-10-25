// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_CAdvanceTime_hpp
#define cf3_solver_CAdvanceTime_hpp

#include "solver/actions/LibActions.hpp"
#include "solver/Action.hpp"

namespace cf3 {
namespace solver {
  class CTime;
namespace actions {

////////////////////////////////////////////////////////////////////////////////

/// CAdvanceTime
/// @author Willem Deconinck
class solver_actions_API CAdvanceTime : public solver::Action {

public: // typedefs

  typedef boost::shared_ptr<CAdvanceTime> Ptr;
  typedef boost::shared_ptr<CAdvanceTime const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CAdvanceTime ( const std::string& name );

  /// Virtual destructor
  virtual ~CAdvanceTime();

  /// Get the class name
  static std::string type_name () { return "CAdvanceTime"; }

  /// Simulates this model
  virtual void execute();

  /// @returns the time component
  solver::CTime& time();

private:

  boost::weak_ptr< solver::CTime > m_time; ///< time used by this action

};

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

#endif // cf3_solver_CAdvanceTime_hpp
