// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Tools_Shell_LibShell_hpp
#define cf3_Tools_Shell_LibShell_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Tools_Shell_API
/// @note build system defines COOLFLUID_TOOLS_SHELL_EXPORTS when compiling
/// CommandLineInterpreter files
#ifdef COOLFLUID_TOOLS_SHELL_EXPORTS
#   define Tools_Shell_API      CF3_EXPORT_API
#   define Tools_Shell_TEMPLATE
#else
#   define Tools_Shell_API      CF3_IMPORT_API
#   define Tools_Shell_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Tools {

/// @brief Classes for command line interpreting %COOLFluiD
///
/// The command line interpreter works interactively, or in batch using file
/// @see coolfluid-command.cpp for implementation
/// @author Willem Deconinck
namespace Shell {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the initialization and termination of the library CommandLineInterpreter
/// @author Tiago Quintino
class Tools_Shell_API LibShell : public common::Library
{
public:

  
  

  /// Constructor
  LibShell ( const std::string& name) : common::Library(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.Tools.Shell"; }

  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "Shell"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the Command Line Interpreter API.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibShell"; }

}; // end LibCommandLineInterpreter

////////////////////////////////////////////////////////////////////////////////

} // Shell
} // Tools
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_Tools_Shell_LibShell_hpp
