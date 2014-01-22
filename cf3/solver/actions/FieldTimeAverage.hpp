// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_FieldTimeAverage_hpp
#define cf3_solver_actions_FieldTimeAverage_hpp

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

class solver_actions_API FieldTimeAverage : public common::Action
{
public: // functions
  /// Contructor
  /// @param name of the component
  FieldTimeAverage ( const std::string& name );

  /// Virtual destructor
  virtual ~FieldTimeAverage() {}

  /// Get the class name
  static std::string type_name () { return "FieldTimeAverage"; }

  /// execute the action
  virtual void execute ();
  
private:
  /// Triggered when the field is updated
  void trigger_field();

  Handle<mesh::Field> m_source_field;
  Handle<mesh::Field> m_statistics_field;

  Uint m_count;
};

/////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_actions_FieldTimeAverage_hpp
