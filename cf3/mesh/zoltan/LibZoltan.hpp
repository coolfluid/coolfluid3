// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_zoltan_LibZoltan_hpp
#define cf3_mesh_zoltan_LibZoltan_hpp

////////////////////////////////////////////////////////////////////////////////

// zoltan includes
#include <mpi.h>
#include <zoltan_cpp.h>
#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro mesh_zoltan_API
/// @note build system defines COOLFLUID_ZOLTAN_EXPORTS when compiling zoltan files
#ifdef COOLFLUID_ZOLTAN_EXPORTS
#   define mesh_zoltan_API      CF3_EXPORT_API
#   define mesh_zoltan_TEMPLATE
#else
#   define mesh_zoltan_API      CF3_IMPORT_API
#   define mesh_zoltan_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
  
/// @brief Library for zoltan mesh partitioning and load balancing
/// @author Willem Deconinck
namespace zoltan {

////////////////////////////////////////////////////////////////////////////////

/// Class defines a mesh partitioner using the zoltan external library
/// @author Willem Deconinck
class mesh_zoltan_API LibZoltan : public common::Library
{
public:

  
  

  /// Constructor
  LibZoltan ( const std::string& name) : common::Library(name) { }

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.mesh.zoltan"; }

  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "zoltan"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements a mesh partitioner using the zoltan external library.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibZoltan"; }
}; // LibZoltan

////////////////////////////////////////////////////////////////////////////////

} // zoltan
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_zoltan_LibZoltan_hpp
