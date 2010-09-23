// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_Core_LibClientUI_hpp
#define CF_GUI_Client_Core_LibClientUI_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/ExportAPI.hpp"
#include "Common/LibraryRegister.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro ClientUI_API
/// @note build system defines ClientUI_EXPORTS when compiling ClientUITools files
#ifdef ClientUI_EXPORTS
#   define ClientUI_API      CF_EXPORT_API
#   define ClientUI_TEMPLATE
#else
#   define ClientUI_API      CF_IMPORT_API
#   define ClientUI_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace GUI {
  /// Basic Classes for ClientUI applications used by CF
  namespace ClientUI {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library ClientUI
  /// @author Tiago Quintino
  class LibClientUI :
      public Common::LibraryRegister<LibClientUI>
  {
  public:

    /// Static function that returns the module name.
    /// Must be implemented for the LibraryRegister template
    /// @return name of the module
    static std::string library_name() { return "ClientUI"; }

    /// Static function that returns the description of the module.
    /// Must be implemented for the LibraryRegister template
    /// @return descripton of the module
    static std::string library_description()
    {
      return "This library implements the ClientUI manipulation API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibClientUI"; }

    /// Start profiling
    virtual void initiate();

    /// Stop profiling
    virtual void terminate();

  }; // end LibClientUI

////////////////////////////////////////////////////////////////////////////////

} // namespace ClientUI
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_Core_LibClientUI_hpp
