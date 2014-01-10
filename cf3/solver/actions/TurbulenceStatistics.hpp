// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_TurbulenceStatistics_hpp
#define cf3_solver_actions_TurbulenceStatistics_hpp

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

class solver_actions_API TurbulenceStatistics : public common::Action
{
public: // functions
  /// Contructor
  /// @param name of the component
  TurbulenceStatistics ( const std::string& name );

  /// Virtual destructor
  virtual ~TurbulenceStatistics() {}

  /// Get the class name
  static std::string type_name () { return "TurbulenceStatistics"; }

  /// execute the action
  virtual void execute ();

  /// Reset the statistics
  void reset_statistics();

  void add_probe(const RealVector& probe_location);
  
private:
  /// Set up the accumulators as configured by the options
  void setup();

  /// Triggered when an option is changed
  void trigger_option();

  void signal_add_probe(common::SignalArgs& args);
  void signature_add_probe(common::SignalArgs& args);
  void signal_setup(common::SignalArgs& args);

  /// Size of the rolling window
  Uint m_rolling_size;
  /// True if one of the options changed since last execute
  bool m_options_changed;
  /// Nodes used by the region
  boost::shared_ptr< common::List<Uint> > m_used_nodes;
  /// Field having the velocity
  Handle<mesh::Field> m_velocity_field;
  Handle<mesh::Field> m_pressure_field;
  Handle<mesh::Field> m_statistics_field;
  Uint m_dim;
  Uint m_velocity_field_offset;
  Uint m_pressure_field_offset;
  
  typedef boost::accumulators::accumulator_set< Real, boost::accumulators::stats<boost::accumulators::tag::mean> > MeanAccT;
  typedef boost::accumulators::accumulator_set< Real, boost::accumulators::stats<boost::accumulators::tag::rolling_mean> > RollingAccT;

  std::vector<MeanAccT> m_means;
  std::vector<RollingAccT> m_rolling_means;
  Uint m_count;
  std::vector<RealVector> m_probe_locations;
  std::vector<Uint> m_probe_nodes;
  std::vector<Uint> m_probe_indices;
  std::vector< boost::shared_ptr<boost::filesystem::fstream> > m_probe_files;
};

/////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_actions_TurbulenceStatistics_hpp
