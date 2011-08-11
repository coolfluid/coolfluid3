// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CAdvanceTime_hpp
#define CF_Solver_CAdvanceTime_hpp

#include "Solver/Actions/LibActions.hpp"
#include "Solver/Action.hpp"

namespace CF {
namespace Solver {
  class CTime;
namespace Actions {

////////////////////////////////////////////////////////////////////////////////

/// CAdvanceTime
/// @author Willem Deconinck
class Solver_Actions_API CAdvanceTime : public Solver::Action {

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
  Solver::CTime& time();

private:

  boost::weak_ptr< Solver::CTime > m_time; ///< time used by this action

};

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

#endif // CF_Solver_CAdvanceTime_hpp
