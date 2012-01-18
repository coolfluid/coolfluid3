// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_FwdEuler_hpp
#define cf3_RDM_FwdEuler_hpp

#include "solver/Action.hpp"

#include "RDM/LibRDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh { class Field; }
namespace RDM {

class RDM_API FwdEuler : public cf3::solver::Action {

public: // typedefs

  /// pointers
  
  

public: // functions
  /// Contructor
  /// @param name of the component
  FwdEuler ( const std::string& name );

  /// Virtual destructor
  virtual ~FwdEuler() {}

  /// Get the class name
  static std::string type_name () { return "FwdEuler"; }

  /// execute the action
  virtual void execute ();

private: // data

  /// solution field pointer
  Handle<mesh::Field> m_solution;
  /// residual field pointer
  Handle<mesh::Field> m_residual;
  /// wave_speed field pointer
  Handle<mesh::Field> m_wave_speed;

};

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3

#endif // cf3_RDM_FwdEuler_hpp
