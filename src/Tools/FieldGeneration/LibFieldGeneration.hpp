// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Tools_FieldGeneration_LibFieldGeneration_hpp
#define CF_Tools_FieldGeneration_LibFieldGeneration_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro FieldGeneration_API
/// @note build system defines COOLFLUID_MESH_GENERATION_EXPORTS when compiling
/// FieldGeneration files
#ifdef COOLFLUID_FIELD_GENERATION_EXPORTS
#   define FieldGeneration_API      CF_EXPORT_API
#   define FieldGeneration_TEMPLATE
#else
#   define FieldGeneration_API      CF_IMPORT_API
#   define FieldGeneration_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {
namespace FieldGeneration {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library FieldGeneration
  /// @author Tiago Quintino
  class FieldGeneration_API LibFieldGeneration :
      public Common::CLibrary
  {
  public:

    typedef boost::shared_ptr<LibFieldGeneration> Ptr;
    typedef boost::shared_ptr<LibFieldGeneration const> ConstPtr;

    /// Constructor
    LibFieldGeneration ( const std::string& name) : Common::CLibrary(name) {   }

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "CF.Tools.FieldGeneration"; }


    /// Static function that returns the library name.
    /// Must be implemented for CLibrary registration
    /// @return name of the library
    static std::string library_name() { return "FieldGeneration"; }

    /// Static function that returns the description of the library.
    /// Must be implemented for CLibrary registration
    /// @return description of the library

    static std::string library_description()
    {
      return "This library implements the FieldGeneration manipulation API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibFieldGeneration"; }

  protected:

    /// initiate library
    virtual void initiate_impl();

    /// terminate library
    virtual void terminate_impl();

  }; // LibFieldGeneration

////////////////////////////////////////////////////////////////////////////////

} // FieldGeneration
} // Tools
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Tools_FieldGeneration_LibFieldGeneration_hpp
