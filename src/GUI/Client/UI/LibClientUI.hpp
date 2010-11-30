// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_UI_Core_LibClientUI_hpp
#define CF_GUI_Client_UI_Core_LibClientUI_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro ClientUI_API
/// @note build system defines COOLFLUID_CLIENT_UI_EXPORTS when compiling ClientUITools files
#ifdef COOLFLUID_CLIENT_UI_EXPORTS
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
  class ClientUI_API LibClientUI :
      public Common::CLibrary
  {
  public:

    typedef boost::shared_ptr<LibClientUI> Ptr;
    typedef boost::shared_ptr<LibClientUI const> ConstPtr;

    /// Constructor
    LibClientUI ( const std::string& name) : Common::CLibrary(name) {   }

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "CF.GUI.ClientUI"; }

    /// Static function that returns the module name.
    /// Must be implemented for CLibrary registration
    /// @return name of the library
    static std::string library_name() { return "ClientUI"; }

    /// Static function that returns the description of the module.
    /// Must be implemented for CLibrary registration
    /// @return description of the library

    static std::string library_description()
    {
      return "This library implements the ClientUI manipulation API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibClientUI"; }

    /// initiate library
    virtual void initiate();

    /// terminate library
    virtual void terminate();

  }; // end LibClientUI

////////////////////////////////////////////////////////////////////////////////

} // ClientUI
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_UI_Core_LibClientUI_hpp
