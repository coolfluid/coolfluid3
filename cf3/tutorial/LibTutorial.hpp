// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_tutorial_LibTutorial_hpp
#define cf3_tutorial_LibTutorial_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro tutorial_API
/// @note build system defines COOLFLUID_TUTORIAL_EXPORTS when compiling tutorial files
#ifdef COOLFLUID_TUTORIAL_EXPORTS
#   define tutorial_API      CF3_EXPORT_API
#   define tutorial_TEMPLATE
#else
#   define tutorial_API      CF3_IMPORT_API
#   define tutorial_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

/// @brief Contains Tutorial classes and functions
namespace tutorial {

////////////////////////////////////////////////////////////////////////////////

/// @brief Defines the Tutorial library
class tutorial_API LibTutorial : public cf3::common::Library 
{
public:

  /// Constructor
  LibTutorial ( const std::string& name) : cf3::common::Library(name) { }

  /// Destructor
  virtual ~LibTutorial() { }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.tutorial"; }

  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "tutorial"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements Tutorial components";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibTutorial"; }
  
  virtual void initiate();
}; // end LibTutorial

////////////////////////////////////////////////////////////////////////////////

} // tutorial
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_tutorial_LibTutorial_hpp
