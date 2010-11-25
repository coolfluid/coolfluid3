// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Tools_MeshDiff_LibMeshDiff_hpp
#define CF_Tools_MeshDiff_LibMeshDiff_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro MeshDiff_API
/// @note build system defines COOLFLUID_MESHDIFF_EXPORTS when compiling
/// MeshDiff files
#ifdef COOLFLUID_MESHDIFF_EXPORTS
#   define MeshDiff_API      CF_EXPORT_API
#   define MeshDiff_TEMPLATE
#else
#   define MeshDiff_API      CF_IMPORT_API
#   define MeshDiff_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {
namespace MeshDiff {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library MeshDiff
  /// @author Tiago Quintino
  class MeshDiff_API LibMeshDiff :
      public Common::CLibrary
  {
  public:

    typedef boost::shared_ptr<LibMeshDiff> Ptr;
    typedef boost::shared_ptr<LibMeshDiff const> ConstPtr;

    /// Constructor
    LibMeshDiff ( const std::string& name) : Common::CLibrary(name) { BUILD_COMPONENT; }

    /// Configuration options
    virtual void define_config_properties () {}

  private: // helper functions

    /// regists all the signals declared in this class
    virtual void define_signals () {}

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "CF.Tools.MeshDiff"; }

    /// Static function that returns the module name.
    /// Must be implemented for CLibrary registration
    /// @return name of the library
    static std::string library_name() { return "MeshDiff"; }

    /// Static function that returns the description of the module.
    /// Must be implemented for CLibrary registration
    /// @return description of the library

    static std::string library_description()
    {
      return "This library implements the MeshDiff manipulation API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibMeshDiff"; }

    /// initiate library
    virtual void initiate();

    /// terminate library
    virtual void terminate();

  }; // end LibMeshDiff

////////////////////////////////////////////////////////////////////////////////

} // MeshDiff
} // Tools
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Tools_MeshDiff_LibMeshDiff_hpp
