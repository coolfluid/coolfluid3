// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_Core_Core_LibClientCore_hpp
#define CF_GUI_Client_Core_Core_LibClientCore_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/ExportAPI.hpp"
#include "Common/LibraryRegister.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro ClientCore_API
/// @note build system defines COOLFLUID_CLIENT_CORE_EXPORTS when compiling ClientCoreTools files
#ifdef COOLFLUID_CLIENT_CORE_EXPORTS
#   define ClientCore_API      CF_EXPORT_API
#   define ClientCore_TEMPLATE
#else
#   define ClientCore_API      CF_IMPORT_API
#   define ClientCore_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace GUI {
/// Basic Classes for client-core library used by coolfluid-client application
namespace ClientCore {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library ClientCore
  /// @author Tiago Quintino
  class ClientCore_API LibClientCore :
      public Common::LibraryRegister<LibClientCore>
  {
  public:

    /// Static function that returns the module name.
    /// Must be implemented for the LibraryRegister template
    /// @return name of the library
    static std::string library_name() { return "ClientCore"; }

    /// Static function that returns the description of the module.
    /// Must be implemented for the LibraryRegister template
    /// @return description of the library

    static std::string library_description()
    {
      return "This library implements the ClientCore manipulation API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibClientCore"; }

    /// initiate library
    virtual void initiate();

    /// terminate library
    virtual void terminate();

  }; // end LibClientCore

////////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_Core_Core_LibClientCore_hpp
