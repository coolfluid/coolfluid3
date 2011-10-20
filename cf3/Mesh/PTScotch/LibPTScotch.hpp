// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_PTScotch_LibPTScotch_hpp
#define CF_Mesh_PTScotch_LibPTScotch_hpp

////////////////////////////////////////////////////////////////////////////////

// ptscotch includes
#include <mpi.h>
#include <ptscotch.h>

#include "common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro PTScotch_API
/// @note build system defines COOLFLUID_PTSCOTCH_EXPORTS when compiling PTScotch files
#ifdef COOLFLUID_NEU_EXPORTS
#   define PTScotch_API      CF_EXPORT_API
#   define PTScotch_TEMPLATE
#else
#   define PTScotch_API      CF_IMPORT_API
#   define PTScotch_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Mesh {
  
/// @brief Library for PTScotch mesh partitioning and load balancing
/// @author Willem Deconinck
namespace PTScotch {

////////////////////////////////////////////////////////////////////////////////

/// Class defines a mesh partitioner using the PTScotch external library
/// @author Willem Deconinck
class PTScotch_API LibPTScotch : public common::CLibrary
{
public:

  typedef boost::shared_ptr<LibPTScotch> Ptr;
  typedef boost::shared_ptr<LibPTScotch const> ConstPtr;

  /// Constructor
  LibPTScotch ( const std::string& name) : common::CLibrary(name) { }

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.Mesh.PTScotch"; }

  /// Static function that returns the module name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "PTScotch"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for CLibrary registration
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

} // PTScotch
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_PTScotch_LibPTScotch_hpp
