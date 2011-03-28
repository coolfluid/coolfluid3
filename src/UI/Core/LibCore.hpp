// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Core_LibUICore_hpp
#define CF_GUI_Core_LibUICore_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Core_API
/// @note build system defines COOLFLUID_UI_CORE_EXPORTS when compiling ClientCoreTools files
#ifdef COOLFLUID_UI_CORE_EXPORTS
#   define Core_API      CF_EXPORT_API
#   define UICore_TEMPLATE
#else
#   define Core_API      CF_IMPORT_API
#   define UICore_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {

namespace UI {
/// Basic Classes for client-core library used by coolfluid-client application
namespace Core {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library ClientCore
  /// @author Tiago Quintino
  class Core_API LibCore :
      public Common::CLibrary
  {
  public:

    typedef boost::shared_ptr<LibCore> Ptr;
    typedef boost::shared_ptr<LibCore const> ConstPtr;

    /// Constructor
    LibCore ( const std::string& name) : Common::CLibrary(name) {   }

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "CF.UI.Core"; }


    /// Static function that returns the module name.
    /// Must be implemented for CLibrary registration
    /// @return name of the library
    static std::string library_name() { return "Core"; }

    /// Static function that returns the description of the module.
    /// Must be implemented for CLibrary registration
    /// @return description of the library

    static std::string library_description()
    {
      return "This library implements the UI Core manipulation API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibCore"; }

    /// initiate library
    virtual void initiate();

    /// terminate library
    virtual void terminate();

  }; // end LibsCore

////////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Core_LibUICore_hpp
