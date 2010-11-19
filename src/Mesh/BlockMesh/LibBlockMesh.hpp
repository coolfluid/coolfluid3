// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_BlockMesh_LibBlockMesh_hpp
#define CF_Mesh_BlockMesh_LibBlockMesh_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/ExportAPI.hpp"
#include "Common/LibraryRegister.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro BlockMesh_API
/// @note build system defines COOLFLUID_BLOCKMESH_EXPORTS when compiling BlockMesh files
#ifdef COOLFLUID_BLOCKMESH_EXPORTS
#   define BlockMesh_API      CF_EXPORT_API
#   define BlockMesh_TEMPLATE
#else
#   define BlockMesh_API      CF_IMPORT_API
#   define BlockMesh_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace BlockMesh {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the BlockMeshtral mesh format operations
/// @author Willem Deconinck
class BlockMesh_API LibBlockMesh :
    public Common::LibraryRegister<LibBlockMesh>
{
public:

  /// Static function that returns the module name.
  /// Must be implemented for the LibraryRegister template
  /// @return name of the library
  static std::string library_name() { return "BlockMesh"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for the LibraryRegister template
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements some BlockMesh compatibility functions.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibBlockMesh"; }

  /// initiate library
  virtual void initiate();

  /// terminate library
  virtual void terminate();
}; // end LibBlockMesh

////////////////////////////////////////////////////////////////////////////////

} // BlockMesh
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_BlockMesh_LibBlockMesh_hpp
