// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CModelUnsteady_hpp
#define CF_Solver_CModelUnsteady_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Solver/CModel.hpp"
#include "Solver/LibSolver.hpp"

namespace CF {
namespace Mesh{
  class Field;
}
namespace Solver {

  class CTime;
  
////////////////////////////////////////////////////////////////////////////////

/// CModelUnsteady models a Unsteady PDE problem
/// @author Tiago Quintino
class Solver_API CModelUnsteady : public Solver::CModel {

public: // typedefs

  typedef boost::shared_ptr<CModelUnsteady> Ptr;
  typedef boost::shared_ptr<CModelUnsteady const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CModelUnsteady ( const std::string& name );

  /// Virtual destructor
  virtual ~CModelUnsteady();

  /// Get the class name
  static std::string type_name () { return "CModelUnsteady"; }

  /// Simulates this model
  virtual void simulate();
  
  /// Create CTime component
  CTime& create_time(const std::string& name = "Time");
  
  /// Signal to create time
  void signal_create_time(Common::SignalArgs node);
  
  /// Reference to the time
  CTime& time();
  
private: // data
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
};

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CModelUnsteady_hpp
