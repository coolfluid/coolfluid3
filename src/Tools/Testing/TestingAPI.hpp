// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_TestingAPI_hpp
#define CF_TestingAPI_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/ExportAPI.hpp"
#include "Common/ModuleRegister.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Testing_API
/// @note build system defines Testing_EXPORTS when compiling TestingTools files
#ifdef Testing_EXPORTS
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
  class TestingLib :
      public Common::ModuleRegister<TestingLib>
  {
  public:

    /// Static function that returns the module name.
    /// Must be implemented for the ModuleRegister template
    /// @return name of the module
    static std::string getModuleName() { return "Testing"; }

    /// Static function that returns the description of the module.
    /// Must be implemented for the ModuleRegister template
    /// @return descripton of the module
    static std::string getModuleDescription()
    {
      return "This library implements the Testing manipulation API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "TestingLib"; }

    /// Start profiling
    virtual void initiate();

    /// Stop profiling
    virtual void terminate();

  }; // end TestingLib

////////////////////////////////////////////////////////////////////////////////

} // namespace Testing
} // namespace Tools
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Testing_hpp
