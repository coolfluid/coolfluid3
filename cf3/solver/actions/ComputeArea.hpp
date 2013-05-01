// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_ComputeArea_hpp
#define cf3_solver_actions_ComputeArea_hpp

#include "solver/actions/LoopOperation.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh { class CScalarFieldView; }
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

class solver_actions_API ComputeArea : public LoopOperation
{
public: // typedefs

  /// pointers
  
  

public: // functions
  /// Contructor
  /// @param name of the component
  ComputeArea ( const std::string& name );

  /// Virtual destructor
  virtual ~ComputeArea() {}

  /// Get the class name
  static std::string type_name () { return "ComputeArea"; }

  /// execute the action
  virtual void execute ();

private: // helper functions

  void config_field();

  void trigger_elements();

private: // data

  Handle<mesh::Field> m_area;
  Handle<mesh::Space const> m_area_field_space;

  RealMatrix m_coordinates;

};

/////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_actions_ComputeArea_hpp
