// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_BlockMesh_LibBlockMesh_hpp
#define cf3_mesh_BlockMesh_LibBlockMesh_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro BlockMesh_API
/// @note build system defines COOLFLUID_BLOCKMESH_EXPORTS when compiling BlockMesh files
#ifdef COOLFLUID_BLOCKMESH_EXPORTS
#   define BlockMesh_API      CF3_EXPORT_API
#   define BlockMesh_TEMPLATE
#else
#   define BlockMesh_API      CF3_IMPORT_API
#   define BlockMesh_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
  
/// @brief Library for I/O of the OpenFOAM BlockMesh dict files
namespace BlockMesh {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the BlockMeshtral mesh format operations
/// @author Willem Deconinck
class BlockMesh_API LibBlockMesh :
    public common::Library
{
public:

  
  

  /// Constructor
  LibBlockMesh ( const std::string& name) : common::Library(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.mesh.BlockMesh"; }

  /// Static function that returns the module name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "BlockMesh"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements some BlockMesh compatibility functions.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibBlockMesh"; }

}; // end LibBlockMesh

////////////////////////////////////////////////////////////////////////////////

} // BlockMesh
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_BlockMesh_LibBlockMesh_hpp
