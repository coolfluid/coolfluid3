// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_ptscotch_LibPTScotch_hpp
#define cf3_mesh_ptscotch_LibPTScotch_hpp

////////////////////////////////////////////////////////////////////////////////

// ptscotch includes
#include <mpi.h>
#include <ptscotch.h>

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro mesh_ptscotch_API
/// @note build system defines COOLFLUID_PTSCOTCH_EXPORTS when compiling PTScotch files
#ifdef COOLFLUID_NEU_EXPORTS
#   define mesh_ptscotch_API      CF3_EXPORT_API
#   define mesh_ptscotch_TEMPLATE
#else
#   define mesh_ptscotch_API      CF3_IMPORT_API
#   define mesh_ptscotch_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

/// @brief Library for PTScotch mesh partitioning and load balancing
/// @author Willem Deconinck
namespace ptscotch {

////////////////////////////////////////////////////////////////////////////////

/// Class defines a mesh partitioner using the PTScotch external library
/// @author Willem Deconinck
class mesh_ptscotch_API LibPTScotch : public common::Library
{
public:

  typedef boost::shared_ptr<LibPTScotch> Ptr;
  typedef boost::shared_ptr<LibPTScotch const> ConstPtr;

  /// Constructor
  LibPTScotch ( const std::string& name) : common::Library(name) { }

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.mesh.ptscotch"; }

  /// Static function that returns the module name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "PTScotch"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements a mesh partitioner using the PTScotch external library.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibPTScotch"; }

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // LibPTScotch

////////////////////////////////////////////////////////////////////////////////

} // ptscotch
} // mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_ptscotch_LibPTScotch_hpp
