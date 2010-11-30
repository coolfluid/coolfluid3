// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Tools_Testing_LibTesting_hpp
#define CF_Tools_Testing_LibTesting_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Testing_API
/// @note build system defines COOLFLUID_TESTING_EXPORTS when compiling
/// Testing files
#ifdef COOLFLUID_TESTING_EXPORTS
#   define Testing_API      CF_EXPORT_API
#   define Testing_TEMPLATE
#else
#   define Testing_API      CF_IMPORT_API
#   define Testing_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {
namespace Testing {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library Testing
  /// @author Tiago Quintino
  class Testing_API LibTesting : public Common::CLibrary
  {
  public:

    typedef boost::shared_ptr<LibTesting> Ptr;
    typedef boost::shared_ptr<LibTesting const> ConstPtr;

    /// Constructor
    LibTesting ( const std::string& name) : Common::CLibrary(name) {   }

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "CF.Tools.Testing"; }

    /// Static function that returns the module name.
    /// Must be implemented for CLibrary registration
    /// @return name of the library
    static std::string library_name() { return "Testing"; }

    /// Static function that returns the description of the module.
    /// Must be implemented for CLibrary registration
    /// @return description of the library

    static std::string library_description()
    {
      return "This library implements the Testing manipulation API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibTesting"; }

    /// initiate library
    virtual void initiate();

    /// terminate library
    virtual void terminate();

  }; // LibTesting

////////////////////////////////////////////////////////////////////////////////

} // Testing
} // Tools
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Tools_Testing_LibTesting_hpp
