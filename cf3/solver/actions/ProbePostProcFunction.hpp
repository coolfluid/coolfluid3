// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_ProbeFunction_hpp
#define cf3_solver_actions_ProbeFunction_hpp

////////////////////////////////////////////////////////////////////////////////

#include "math/AnalyticalFunction.hpp"
#include "solver/actions/Probe.hpp"

namespace cf3 {
namespace solver {
namespace actions {

////////////////////////////////////////////////////////////////////////////////

/// @brief Probe Post Processor class to compute a configurable function
///
/// This allows to add a dynamic function to the probe
/// @author Willem Deconinck
class solver_actions_API ProbePostProcFunction : public ProbePostProcessor {
public:

  /// Contructor
  /// @param name of the component
  ProbePostProcFunction ( const std::string& name );

  /// Virtual destructor
  virtual ~ProbePostProcFunction() {}

  /// Get the class name
  static std::string type_name () { return "ProbePostProcFunction"; }

  virtual void execute();

  void update_function();
  void update_params();

private:

  math::AnalyticalFunction function;

  std::string m_var_str;
  std::string m_function_str;

  std::vector<Real> m_params;
};

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_ProbeFunction_hpp
