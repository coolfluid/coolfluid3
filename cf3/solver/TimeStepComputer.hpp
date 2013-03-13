// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_TimeStepComputer_hpp
#define cf3_solver_TimeStepComputer_hpp

#include "common/Action.hpp"
#include "solver/LibSolver.hpp"

/////////////////////////////////////////////////////////////////////////////////////

// Forward declarations
namespace cf3 {
  namespace mesh { 
    class Field; 
  }
  namespace solver { 
    class Time;   
  }
}

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {

/// @brief Base class to compute a time_step field
/// @author Willem Deconinck
/// The time_step is returned as a field, in case for 
/// local time-stepping for non-timeaccurate solvers
class solver_API TimeStepComputer : public common::Action
{
public: // functions
  /// Contructor
  /// @param name of the component
  TimeStepComputer ( const std::string& name );

  /// Virtual destructor
  virtual ~TimeStepComputer() {};

  /// Get the class name
  static std::string type_name () { return "TimeStepComputer"; }

  virtual void change_with_factor(const Real& factor) = 0;

  virtual const Real& max_cfl() const = 0;

protected: // data

  Handle<mesh::Field>  m_time_step;
  Handle<mesh::Field>  m_wave_speed;
  Handle<Time> m_time;
};

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_TimeStepComputer_hpp
