// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Tools_MeshGeneration_LibMeshGeneration_hpp
#define cf3_Tools_MeshGeneration_LibMeshGeneration_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro MeshGeneration_API
/// @note build system defines COOLFLUID_MESH_GENERATION_EXPORTS when compiling
/// MeshGeneration files
#ifdef COOLFLUID_MESH_GENERATION_EXPORTS
#   define MeshGeneration_API      CF3_EXPORT_API
#   define MeshGeneration_TEMPLATE
#else
#   define MeshGeneration_API      CF3_IMPORT_API
#   define MeshGeneration_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Tools {
namespace MeshGeneration {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library MeshGeneration
  /// @author Tiago Quintino
  class MeshGeneration_API LibMeshGeneration :
      public common::Library
  {
  public:

    
    

    /// Constructor
    LibMeshGeneration ( const std::string& name) : common::Library(name) {   }

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "cf3.Tools.MeshGeneration"; }


    /// Static function that returns the library name.
    /// Must be implemented for Library registration
    /// @return name of the library
    static std::string library_name() { return "MeshGeneration"; }

    /// Static function that returns the description of the library.
    /// Must be implemented for Library registration
    /// @return description of the library

    static std::string library_description()
    {
      return "This library implements the MeshGeneration manipulation API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibMeshGeneration"; }

  }; // LibMeshGeneration

////////////////////////////////////////////////////////////////////////////////

} // MeshGeneration
} // Tools
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Tools_MeshGeneration_LibMeshGeneration_hpp
