// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_LibVTKLegacy_hpp
#define cf3_LibVTKLegacy_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro VTKLegacy_API
/// @note build system defines COOLFLUID_GMSH_EXPORTS when compiling VTKLegacy files
#ifdef COOLFLUID_GMSH_EXPORTS
#   define VTKLegacy_API      CF3_EXPORT_API
#   define VTKLegacy_TEMPLATE
#else
#   define VTKLegacy_API      CF3_IMPORT_API
#   define VTKLegacy_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Mesh {
  
/// @brief Library for I/O of the VTK legacy format
namespace VTKLegacy {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the VTKLegacytral mesh format operations
/// @author Willem Deconinck
class VTKLegacy_API LibVTKLegacy :
    public common::CLibrary
{
public:

  typedef boost::shared_ptr<LibVTKLegacy> Ptr;
  typedef boost::shared_ptr<LibVTKLegacy const> ConstPtr;

  /// Constructor
  LibVTKLegacy ( const std::string& name) : common::CLibrary(name) {   }

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.Mesh.VTKLegacy"; }

  /// Static function that returns the library name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "VTKLegacy"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the VTKLegacy mesh format operations.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibVTKLegacy"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibVTKLegacy

////////////////////////////////////////////////////////////////////////////////

} // VTKLegacy
} // Mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // CF3_LibVTKLegacy_hpp
