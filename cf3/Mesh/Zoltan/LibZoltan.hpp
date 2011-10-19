// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Mesh_Zoltan_LibZoltan_hpp
#define cf3_Mesh_Zoltan_LibZoltan_hpp

////////////////////////////////////////////////////////////////////////////////

// zoltan includes
#include <mpi.h>
#include <zoltan_cpp.h>

// typedef for namespace, object conflict
typedef Zoltan ZoltanHandle;

#include "common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Zoltan_API
/// @note build system defines COOLFLUID_ZOLTAN_EXPORTS when compiling Zoltan files
#ifdef COOLFLUID_NEU_EXPORTS
#   define Zoltan_API      CF3_EXPORT_API
#   define Zoltan_TEMPLATE
#else
#   define Zoltan_API      CF3_IMPORT_API
#   define Zoltan_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Mesh {
  
/// @brief Library for Zoltan mesh partitioning and load balancing
/// @author Willem Deconinck
namespace Zoltan {

////////////////////////////////////////////////////////////////////////////////

/// Class defines a mesh partitioner using the Zoltan external library
/// @author Willem Deconinck
class Zoltan_API LibZoltan : public common::CLibrary
{
public:

  typedef boost::shared_ptr<LibZoltan> Ptr;
  typedef boost::shared_ptr<LibZoltan const> ConstPtr;

  /// Constructor
  LibZoltan ( const std::string& name) : common::CLibrary(name) { }

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.Mesh.Zoltan"; }

  /// Static function that returns the library name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "Zoltan"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements a mesh partitioner using the Zoltan external library.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibZoltan"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // LibZoltan

////////////////////////////////////////////////////////////////////////////////

} // Zoltan
} // Mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Mesh_Zoltan_LibZoltan_hpp
