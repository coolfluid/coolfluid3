// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_TimeSeriesWriter_hpp
#define cf3_solver_actions_TimeSeriesWriter_hpp

#include "common/Action.hpp"
#include "solver/actions/LibActions.hpp"

#include "solver/Time.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

/// Helper class to write out a time series. It loops through all child actions having a "file" option
///  (i.e. mesh writers) and uses this as a template for a filename containing time step information.
/// Filename templates can include {time} (with the{}) to include the current timestep and
/// {iteration} to include the current iteration number
/// The interval option controls the number of timesteps after which a solution is to be written
class solver_actions_API TimeSeriesWriter : public common::Action
{
public: // functions
  /// Contructor
  /// @param name of the component
  TimeSeriesWriter ( const std::string& name );

  /// Virtual destructor
  virtual ~TimeSeriesWriter() {}

  /// Get the class name
  static std::string type_name () { return "TimeSeriesWriter"; }

  /// execute the action
  virtual void execute();
private:
  Handle<Time> m_time;
  Uint m_interval;
};

/////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_actions_TimeSeriesWriter_hpp
