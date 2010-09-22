// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Network_LibNetwork_hpp
#define CF_GUI_Network_LibNetwork_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/ExportAPI.hpp"
#include "Common/LibraryRegister.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Network_API
/// @note build system defines Network_EXPORTS when compiling Network files
#ifdef Network_EXPORTS
#   define Network_API CF_EXPORT_API
#else
#   define Network_API CF_IMPORT_API
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace GUI {

  /// Common classes for the client and the server
  namespace Network {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library Network
  /// @author Tiago Quintino
  class LibNetwork :
      public Common::LibraryRegister<LibNetwork>
  {
  public:

    /// Static function that returns the module name.
    /// Must be implemented for the LibraryRegister template
    /// @return name of the module
    static std::string library_name() { return "Network"; }

    /// Static function that returns the description of the module.
    /// Must be implemented for the LibraryRegister template
    /// @return descripton of the module
    static std::string library_description()
    {
      return "This library implements the Network manipulation API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibNetwork"; }

    /// Start profiling
    virtual void initiate();

    /// Stop profiling
    virtual void terminate();

  }; // end LibNetwork

////////////////////////////////////////////////////////////////////////////////

} // Network
} // GUI
} // CF

#endif // CF_GUI_Network_LibNetwork_hpp
