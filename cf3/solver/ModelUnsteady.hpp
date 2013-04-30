// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_ModelUnsteady_hpp
#define cf3_solver_ModelUnsteady_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/Model.hpp"
#include "solver/LibSolver.hpp"

namespace cf3 {
namespace mesh{
  class Field;
}
namespace solver {

  class Time;
  
////////////////////////////////////////////////////////////////////////////////

/// ModelUnsteady models a Unsteady PDE problem
/// @author Tiago Quintino
class solver_API ModelUnsteady : public solver::Model {

public: // functions

  /// Contructor
  /// @param name of the component
  ModelUnsteady ( const std::string& name );

  /// Virtual destructor
  virtual ~ModelUnsteady();

  /// Get the class name
  static std::string type_name () { return "ModelUnsteady"; }

  /// Expand short setup with time creation
  virtual void setup(const std::string& solver_builder_name, const std::string& physics_builder_name);

  /// Simulates this model
  virtual void simulate();
  
  /// Create Time component
  Time& create_time(const std::string& name = "Time");
  
  /// Signal to create time
  void signal_create_time(common::SignalArgs& node);
  
  /// Reference to the time
  Time& time();
  
private: // data
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
};

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_ModelUnsteady_hpp
