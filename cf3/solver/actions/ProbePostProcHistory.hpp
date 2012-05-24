// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_ProbePostProcHistory_hpp
#define cf3_solver_actions_ProbePostProcHistory_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/actions/Probe.hpp"

namespace cf3 {
namespace solver {
  class History;

namespace actions {

////////////////////////////////////////////////////////////////////////////////

/// @brief ProbePostProcHistory class to attach to a probe
///
/// This allows to log probed variables to a solver::History component
/// @author Willem Deconinck
class solver_actions_API ProbePostProcHistory : public ProbePostProcessor {
public:

  /// Contructor
  /// @param name of the component
  ProbePostProcHistory ( const std::string& name );

  /// Virtual destructor
  virtual ~ProbePostProcHistory() {}

  /// Get the class name
  static std::string type_name () { return "ProbePostProcHistory"; }

  virtual void execute();

private:

  Handle<History> m_history;
  std::vector<std::string> m_vars;

};

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_actions_ProbePostProcHistory_hpp
