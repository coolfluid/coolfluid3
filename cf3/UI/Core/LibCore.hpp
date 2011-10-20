// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Core_LibUICore_hpp
#define cf3_GUI_Core_LibUICore_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Core_API
/// @note build system defines COOLFLUID_UI_CORE_EXPORTS when compiling ClientCoreTools files
#ifdef COOLFLUID_UI_CORE_EXPORTS
#   define Core_API      CF3_EXPORT_API
#   define Core_TEMPLATE
#else
#   define Core_API      CF3_IMPORT_API
#   define Core_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace UI {
/// Basic Classes for client-core library used by coolfluid-client application
namespace Core {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library ClientCore
  /// @author Tiago Quintino
  class Core_API LibCore :
      public common::CLibrary
  {
  public:

    typedef boost::shared_ptr<LibCore> Ptr;
    typedef boost::shared_ptr<LibCore const> ConstPtr;

    /// Constructor
    LibCore ( const std::string& name) : common::CLibrary(name) {   }

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "CF.UI.Core"; }


    /// Static function that returns the library name.
    /// Must be implemented for CLibrary registration
    /// @return name of the library
    static std::string library_name() { return "Core"; }

    /// Static function that returns the description of the library.
    /// Must be implemented for CLibrary registration
    /// @return description of the library

    static std::string library_description()
    {
      return "This library implements the UI Core manipulation API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibCore"; }

  protected:

    /// initiate library
    virtual void initiate_impl();

    /// terminate library
    virtual void terminate_impl();

  }; // end LibsCore

////////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_GUI_Core_LibUICore_hpp
