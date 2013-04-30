// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_LSSAPI_hpp
#define cf3_LSSAPI_hpp

////////////////////////////////////////////////////////////////////////////////


#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro LSS_API
#ifdef COOLFLUID_MATH_EXPORTS
#   define LSS_API      CF3_EXPORT_API
#   define Math_TEMPLATE
#else
#   define LSS_API      CF3_IMPORT_API
#   define Math_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {
namespace LSS {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library LSS
  class LSS_API LibLSS :  public common::Library
  {
  public:
    /// Constructor
    LibLSS ( const std::string& name) : common::Library(name) {   }

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "cf3.math.LSS"; }

    /// Static function that returns the library name.
    /// Must be implemented for Library registration
    /// @return name of the library
    static std::string library_name() { return "math"; }

    /// Static function that returns the description of the library.
    /// Must be implemented for Library registration
    /// @return description of the library

    static std::string library_description()
    {
      return "This library implements the MeshDiff manipulation API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibLSS"; }
  }; // end LibLSS

////////////////////////////////////////////////////////////////////////////////


} // LSS
} // math
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_LSSAPI_hpp
