// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Tools_MeshDiff_LibMeshDiff_hpp
#define cf3_Tools_MeshDiff_LibMeshDiff_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro MeshDiff_API
/// @note build system defines COOLFLUID_MESHDIFF_EXPORTS when compiling
/// MeshDiff files
#ifdef COOLFLUID_MESHDIFF_EXPORTS
#   define MeshDiff_API      CF3_EXPORT_API
#   define MeshDiff_TEMPLATE
#else
#   define MeshDiff_API      CF3_IMPORT_API
#   define MeshDiff_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Tools {
namespace MeshDiff {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library MeshDiff
  /// @author Tiago Quintino
  class MeshDiff_API LibMeshDiff :
      public common::CLibrary
  {
  public:

    typedef boost::shared_ptr<LibMeshDiff> Ptr;
    typedef boost::shared_ptr<LibMeshDiff const> ConstPtr;

    /// Constructor
    LibMeshDiff ( const std::string& name) : common::CLibrary(name) {   }

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "CF.Tools.MeshDiff"; }

    /// Static function that returns the library name.
    /// Must be implemented for CLibrary registration
    /// @return name of the library
    static std::string library_name() { return "MeshDiff"; }

    /// Static function that returns the description of the library.
    /// Must be implemented for CLibrary registration
    /// @return description of the library

    static std::string library_description()
    {
      return "This library implements the MeshDiff manipulation API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibMeshDiff"; }

  protected:

    /// initiate library
    virtual void initiate_impl();

    /// terminate library
    virtual void terminate_impl();

  }; // end LibMeshDiff

////////////////////////////////////////////////////////////////////////////////

} // MeshDiff
} // Tools
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Tools_MeshDiff_LibMeshDiff_hpp
