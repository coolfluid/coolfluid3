// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_ComputeVolume_hpp
#define cf3_solver_actions_ComputeVolume_hpp

#include "solver/actions/LoopOperation.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
  class Elements;
  class Field;
  class CScalarFieldView;
  class Space;
}
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

class solver_actions_API ComputeVolume : public LoopOperation {

public: // typedefs

  /// pointers
  
  

public: // functions
  /// Contructor
  /// @param name of the component
  ComputeVolume ( const std::string& name );

  /// Virtual destructor
  virtual ~ComputeVolume() {}

  /// Get the class name
  static std::string type_name () { return "ComputeVolume"; }

  /// execute the action
  virtual void execute ();

private: // helper functions

  void config_field();

  void trigger_elements();

private: // data

  Handle<mesh::Field> m_volume;
  Handle<mesh::Space const> m_volume_field_space;

  RealMatrix m_coordinates;

};

/////////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3

/////////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_actions_ComputeVolume_hpp
