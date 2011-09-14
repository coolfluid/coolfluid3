// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_LSSAPI_hpp
#define CF_LSSAPI_hpp

////////////////////////////////////////////////////////////////////////////////


#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro LSS_API
#ifdef COOLFLUID_MATH_EXPORTS
#   define LSS_API      CF_EXPORT_API
#   define Math_TEMPLATE
#else
#   define LSS_API      CF_IMPORT_API
#   define Math_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Math {
namespace LSS {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library LSS
  class LSS_API LibLSS :  public Common::CLibrary
  {
  public:

    typedef boost::shared_ptr<LibLSS> Ptr;
    typedef boost::shared_ptr<LibLSS const> ConstPtr;

    /// Constructor
    LibLSS ( const std::string& name) : Common::CLibrary(name) {   }

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "CF.Math.LSS"; }

    /// Static function that returns the library name.
    /// Must be implemented for CLibrary registration
    /// @return name of the library
    static std::string library_name() { return "Math"; }

    /// Static function that returns the description of the library.
    /// Must be implemented for CLibrary registration
    /// @return description of the library

    static std::string library_description()
    {
      return "This library implements the MeshDiff manipulation API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibLSS"; }

  protected:

    /// initiate library
    virtual void initiate_impl();

    /// terminate library
    virtual void terminate_impl();

  }; // end LibLSS

////////////////////////////////////////////////////////////////////////////////


} // LSS
} // Math
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_LSSAPI_hpp
