// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_uiCommon_LibUICommon_hpp
#define cf3_ui_uiCommon_LibUICommon_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro uiCommon_API
/// @note build system defines COOLFLuiD_uiCommon_EXPORTS when compiling Network files
#ifdef COOLFLuiD_uiCOMMONs_EXPORTS
#   define uiCommon_API CF3_EXPORT_API
#   define uiCommon_TEMPLATE
#else
#   define uiCommon_API CF3_IMPORT_API
#   define uiCommon_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {

/// Common classes for the client and the server
namespace uiCommon {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library Network
  /// @author Tiago QUintino
  class uiCommon_API LibUICommon :
      public common::Library
  {
  public:

    
    

    /// Constructor
    LibUICommon ( const std::string& name) : common::Library(name) {   }

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "cf3.ui.uiCommon"; }

    /// Static function that returns the library name.
    /// Must be implemented for Library registration
    /// @return name of the library
    static std::string library_name() { return "LibUICommon"; }

    /// Static function that returns the description of the library.
    /// Must be implemented for Library registration
    /// @return description of the library

    static std::string library_description()
    {
      return "This library provides some common code for ui libraries and applications.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibuiConmon"; }
  }; // end LibUICommon

////////////////////////////////////////////////////////////////////////////////

} // Network
} // ui
} // cf3

#endif // cf3_ui_uiCommon_LibUICommon_hpp
