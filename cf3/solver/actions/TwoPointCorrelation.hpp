// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_TwoPointCorrelation_hpp
#define cf3_solver_actions_TwoPointCorrelation_hpp

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

/// Compute two-point correlations in two perpendipular directions on a structured mesh
class solver_actions_API TwoPointCorrelation : public common::Action
{
public: // functions
  /// Contructor
  /// @param name of the component
  TwoPointCorrelation ( const std::string& name );

  /// Virtual destructor
  virtual ~TwoPointCorrelation() {}

  /// Get the class name
  static std::string type_name () { return "TwoPointCorrelation"; }

  /// execute the action
  virtual void execute ();

  
private:
  void trigger();
  void setup();
  Handle<mesh::Field> m_field;
  
  std::vector<Uint> m_used_node_lids;
  std::vector<Uint> m_used_node_x_gids;
  std::vector<Uint> m_used_node_y_gids;
  std::vector<Real> m_x_positions;
  std::vector<Real> m_y_positions;
  
  RealMatrix m_x_corr;
  RealMatrix m_y_corr;

  Uint m_root;
  std::vector<Uint> m_gids;
  std::vector<Uint> m_ranks;
  
  common::Table<Real>::ArrayT m_sampled_values;

  Handle<common::PE::CommPattern> m_comm_pattern;
  
  Uint m_count;
  Uint m_interval;
};

/////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_actions_TwoPointCorrelation_hpp
