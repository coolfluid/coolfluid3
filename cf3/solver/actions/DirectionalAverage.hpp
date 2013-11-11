// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_DirectionalAverage_hpp
#define cf3_solver_actions_DirectionalAverage_hpp

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/rolling_mean.hpp>

#include "common/Action.hpp"
#include "common/List.hpp"

#include "mesh/Field.hpp"

#include "solver/actions/LibActions.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

/// Take the average along planes perpendicular to the given direction
/// Assumes a structured mesh
class solver_actions_API DirectionalAverage : public common::Action
{
public: // functions
  /// Contructor
  /// @param name of the component
  DirectionalAverage ( const std::string& name );

  /// Virtual destructor
  virtual ~DirectionalAverage() {}

  /// Get the class name
  static std::string type_name () { return "DirectionalAverage"; }

  /// execute the action
  virtual void execute ();

  
private:
  void trigger();
  void setup();
  Handle<mesh::Field> m_field;
  std::vector<Uint> m_node_position_indices;
  std::vector<Real> m_positions;
  std::vector<Real> m_averages;
};

/////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_actions_DirectionalAverage_hpp
