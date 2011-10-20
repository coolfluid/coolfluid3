// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Tools_Testing_LibTesting_hpp
#define cf3_Tools_Testing_LibTesting_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Testing_API
/// @note build system defines COOLFLUID_TESTING_EXPORTS when compiling
/// Testing files
#ifdef COOLFLUID_TESTING_EXPORTS
#   define Testing_API      CF3_EXPORT_API
#   define Testing_TEMPLATE
#else
#   define Testing_API      CF3_IMPORT_API
#   define Testing_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Tools {
namespace Testing {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library Testing
  /// @author Tiago Quintino
  class Testing_API LibTesting : public common::CLibrary
  {
  public:

    typedef boost::shared_ptr<LibTesting> Ptr;
    typedef boost::shared_ptr<LibTesting const> ConstPtr;

    /// Constructor
    LibTesting ( const std::string& name) : common::CLibrary(name) {   }

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "CF.Tools.Testing"; }

    /// Static function that returns the library name.
    /// Must be implemented for CLibrary registration
    /// @return name of the library
    static std::string library_name() { return "Testing"; }

    /// Static function that returns the description of the library.
    /// Must be implemented for CLibrary registration
    /// @return description of the library

    static std::string library_description()
    {
      return "This library implements the Testing manipulation API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibTesting"; }

  protected:

    /// initiate library
    virtual void initiate_impl();

    /// terminate library
    virtual void terminate_impl();

  }; // LibTesting

////////////////////////////////////////////////////////////////////////////////

} // Testing
} // Tools
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Tools_Testing_LibTesting_hpp
