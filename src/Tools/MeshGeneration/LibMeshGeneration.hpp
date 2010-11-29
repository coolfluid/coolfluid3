// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Tools_MeshGeneration_LibMeshGeneration_hpp
#define CF_Tools_MeshGeneration_LibMeshGeneration_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro MeshGeneration_API
/// @note build system defines COOLFLUID_MESH_GENERATION_EXPORTS when compiling
/// MeshGeneration files
#ifdef COOLFLUID_MESH_GENERATION_EXPORTS
#   define MeshGeneration_API      CF_EXPORT_API
#   define MeshGeneration_TEMPLATE
#else
#   define MeshGeneration_API      CF_IMPORT_API
#   define MeshGeneration_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {
namespace MeshGeneration {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library MeshGeneration
  /// @author Tiago Quintino
  class MeshGeneration_API LibMeshGeneration :
      public Common::CLibrary
  {
  public:

    typedef boost::shared_ptr<LibMeshGeneration> Ptr;
    typedef boost::shared_ptr<LibMeshGeneration const> ConstPtr;

    /// Constructor
    LibMeshGeneration ( const std::string& name) : Common::CLibrary(name) { tag_component(this); }

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "CF.Tools.MeshGeneration"; }


    /// Static function that returns the module name.
    /// Must be implemented for CLibrary registration
    /// @return name of the library
    static std::string library_name() { return "MeshGeneration"; }

    /// Static function that returns the description of the module.
    /// Must be implemented for CLibrary registration
    /// @return description of the library

    static std::string library_description()
    {
      return "This library implements the MeshGeneration manipulation API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibMeshGeneration"; }

    /// initiate library
    virtual void initiate();

    /// terminate library
    virtual void terminate();

  }; // LibMeshGeneration

////////////////////////////////////////////////////////////////////////////////

} // MeshGeneration
} // Tools
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Tools_MeshGeneration_LibMeshGeneration_hpp
