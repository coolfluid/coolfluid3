// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_CModelUnsteady_hpp
#define cf3_solver_CModelUnsteady_hpp

////////////////////////////////////////////////////////////////////////////////

#include "solver/CModel.hpp"
#include "solver/LibSolver.hpp"

namespace cf3 {
namespace mesh{
  class Field;
}
namespace solver {

  class CTime;
  
////////////////////////////////////////////////////////////////////////////////

/// CModelUnsteady models a Unsteady PDE problem
/// @author Tiago Quintino
class solver_API CModelUnsteady : public solver::CModel {

public: // typedefs

  
  

public: // functions

  /// Contructor
  /// @param name of the component
  CModelUnsteady ( const std::string& name );

  /// Virtual destructor
  virtual ~CModelUnsteady();

  /// Get the class name
  static std::string type_name () { return "CModelUnsteady"; }

  /// Expand short setup with time creation
  virtual void setup(const std::string& solver_builder_name, const std::string& physics_builder_name);

  /// Simulates this model
  virtual void simulate();
  
  /// Create CTime component
  CTime& create_time(const std::string& name = "Time");
  
  /// Signal to create time
  void signal_create_time(common::SignalArgs node);
  
  /// Reference to the time
  CTime& time();
  
private: // data
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
};

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_CModelUnsteady_hpp
