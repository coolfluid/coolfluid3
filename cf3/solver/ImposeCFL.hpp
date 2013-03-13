// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_ImposeCFL_hpp
#define cf3_solver_ImposeCFL_hpp

#include "solver/TimeStepComputer.hpp"
#include "math/AnalyticalFunction.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {

class solver_API ImposeCFL : public solver::TimeStepComputer
{
public: // functions
  /// Contructor
  /// @param name of the component
  ImposeCFL ( const std::string& name );

  /// Virtual destructor
  virtual ~ImposeCFL() {};

  /// Get the class name
  static std::string type_name () { return "ImposeCFL"; }

  /// execute the action
  virtual void execute ();

  void change_with_factor(const Real& factor);

  virtual const Real& max_cfl() const { return m_cfl; }

private:

  void parse_cfl();

  math::AnalyticalFunction m_cfl_function;

  Real m_cfl;

};

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_ImposeCFL_hpp
