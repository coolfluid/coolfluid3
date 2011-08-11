// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_LibSF_hpp
#define CF_Mesh_LibSF_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro SF_API
/// @note build system defines COOLFLUID_MESH_SF_EXPORTS when compiling SF files
#ifdef COOLFLUID_MESH_SF_EXPORTS
#   define MESH_SF_API      CF_EXPORT_API
#   define MESH_SF_TEMPLATE
#else
#   define MESH_SF_API      CF_IMPORT_API
#   define MESH_SF_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  
/// @brief Library for element types and shape functions
/// @note Of course other element types and shape functions can be added in other
///       libraries as well.
/// @author Willem Deconinck
namespace SF {

////////////////////////////////////////////////////////////////////////////////

  /// Shape functions module
  /// @author Tiago Quintino, Willem Deconinck, Bart Janssens
  class MESH_SF_API LibSF : public Common::CLibrary
  {
  public:

    typedef boost::shared_ptr<LibSF> Ptr;
    typedef boost::shared_ptr<LibSF const> ConstPtr;

    /// Constructor
    LibSF ( const std::string& name) : Common::CLibrary(name) {   }

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "CF.Mesh.SF"; }


    /// Static function that returns the library name.
    /// Must be implemented for CLibrary registration
    /// @return name of the library
    static std::string library_name() { return "SF"; }

    /// Static function that returns the description of the library.
    /// Must be implemented for CLibrary registration
    /// @return description of the library

    static std::string library_description()
    {
      return "This library implements the shape functions.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibSF"; }

  protected:

    /// initiate library
    virtual void initiate_impl();

    /// terminate library
    virtual void terminate_impl();

  }; // end LibSF

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_LibSF_hpp
