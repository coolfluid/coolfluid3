// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_mesh_actions_LibActions_hpp
#define cf3_mesh_mesh_actions_LibActions_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro namespace actions_API
/// @note build system defines COOLFLUID_ACTIONS_EXPORTS when compiling MeshTools files
#ifdef COOLFLUID_MESH_ACTIONS_EXPORTS
#   define mesh_actions_API      CF3_EXPORT_API
#   define mesh_actions_TEMPLATE
#else
#   define mesh_actions_API      CF3_IMPORT_API
#   define mesh_actions_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh{

/// @brief Action derived classes for mesh manipulations
namespace actions {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the initialization and termination of the library actions
class mesh_actions_API LibActions :
    public common::Library {

public:

  
  

  /// Constructor
  LibActions ( const std::string& name) : common::Library(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.mesh.actions"; }


  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "actions"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library
  static std::string library_description()
  {
    return "This library implements several Mesh actions.";
  }

  /// Gets the Class name
  static std::string type_name() { return "mesh::actions::LibActions"; }

}; // end mesh::actions::LibActions

////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_mesh_actions_LibActions_hpp
