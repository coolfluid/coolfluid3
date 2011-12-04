// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_RK_hpp
#define cf3_RDM_RK_hpp

#include "solver/Action.hpp"

#include "RDM/LibRDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh { class Field; }
namespace RDM {

/// Runge-Kutta update step
/// @author Tiago Quintino
class RDM_API RK : public cf3::solver::Action {

public: // typedefs

  /// pointers
  
  

public: // functions
  /// Contructor
  /// @param name of the component
  RK ( const std::string& name );

  /// Virtual destructor
  virtual ~RK() {}

  /// Get the class name
  static std::string type_name () { return "RK"; }

  /// execute the action
  virtual void execute ();

private: // data

  /// solution field pointer
  Handle<mesh::Field> m_solution;
  /// residual field pointer
  Handle<mesh::Field> m_residual;
  /// dual_area field pointer
  Handle<mesh::Field> m_dual_area;

};

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3

#endif // cf3_RDM_RK_hpp
