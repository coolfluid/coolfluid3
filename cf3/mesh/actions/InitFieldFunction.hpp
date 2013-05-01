// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_InitFieldFunction_hpp
#define cf3_mesh_InitFieldFunction_hpp

////////////////////////////////////////////////////////////////////////////////

#include "math/VectorialFunction.hpp"

#include "mesh/MeshTransformer.hpp"

#include "mesh/actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh { 
  class Field;
namespace actions {

//////////////////////////////////////////////////////////////////////////////

/// @brief Initialize a field with given functions
/// @author Willem Deconinck
/// The functions can contain as variables any variable defined in the dictionary
/// the field belongs to.
/// Append vector-field variables with "x" or "y" or "z".
/// Append tensor-field variables with "xx", "xy", "xz", "yx", ...
/// Append array-field variables with "[0]", "[1]", "[2]", ...
/// The coordinate field can just be used as "x", "y", "z".
class mesh_actions_API InitFieldFunction : public MeshTransformer
{
public: // functions
  
  /// constructor
  InitFieldFunction( const std::string& name );
  
  /// destructor
  virtual ~InitFieldFunction();
  
  /// Gets the Class name
  static std::string type_name() { return "InitFieldFunction"; }

  // execute
  virtual void execute();

  /// Signal/Signature to configure and execute()
  void signal_init_field( common::SignalArgs& args );

  /// Signal/Signature to configure and execute()
  void signature_init_field( common::SignalArgs& args );

private: // functions

  void config_function();

  void add_variable_with_suffix(std::vector<std::string>& names,
                                std::vector< std::pair<std::string,std::string> >& replace,
                                const std::string& name,
                                const std::string& suffix);
private: // data
  
  math::VectorialFunction  m_function;
  
  Handle<Field> m_field;
  
}; // end InitFieldFunction


////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_InitFieldFunction_hpp
