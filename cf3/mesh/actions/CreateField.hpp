// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_actions_CreateField_hpp
#define cf3_mesh_actions_CreateField_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/MeshTransformer.hpp"
#include "mesh/actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

  class Dictionary;
  class Field;

namespace actions {

//////////////////////////////////////////////////////////////////////////////

/// @brief Create a field with given functions
///
/// Given functions are analytic and can use other functions as variables\n
/// Example function, assuming that a field "myfield" exists with 4 variables,
/// as well as a "coordinates" field in the same dictionary:\n
/// @verbatim
/// coordinates[0]*myfield[2]^myfield[1]
/// @endverbatim
///
/// @author Willem Deconinck
class mesh_actions_API CreateField : public MeshTransformer
{   
public: // functions
  
  /// constructor
  CreateField( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "CreateField"; }

  Handle<Field> create_field(const std::string& name, Dictionary& dict, const std::vector<std::string>& function_str);

  virtual void execute();
  
public: // signals

  void signal_create_field( common::SignalArgs& args);
  void signature_create_field( common::SignalArgs& args);

private: // functions

  void replace_var_name (const std::string& var_from,
                         const std::string& var_to,
                         std::vector<std::string>& functions);

  /// @brief Split a list of functions in a vector of functions
  ///
  /// @param function_list  List of the form "[func1,func2,...]"
  /// @return vector of func1, func2, ...
  std::vector<std::string> split_function_list(const std::string& function_list);

}; // end CreateField

////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_actions_CreateField_hpp
