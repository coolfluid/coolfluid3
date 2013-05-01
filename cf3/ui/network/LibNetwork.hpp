// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_network_lib_network_hpp
#define cf3_ui_network_lib_network_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Network_API
/// @note build system defines COOLFLUID_NETWORK_EXPORTS when compiling Network files
#ifdef COOLFLUID_UI_NETWORK_EXPORTS
#   define Network_API      CF3_EXPORT_API
#   define Network_TEMPLATE
#else
#   define Network_API      CF3_IMPORT_API
#   define Network_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace ui {

/// Network layer for XML communications
namespace network {

////////////////////////////////////////////////////////////////////////////////

    /// Class defines the initialization and termination of the library Network
    /// @author Tiago Quintino
    class Network_API LibNetwork :  public common::Library
    {
    public:
      /// Constructor
      LibNetwork ( const std::string& name) : common::Library(name) {   }

    public: // functions

      /// @return string of the library namespace
      static std::string library_namespace() { return "cf3.ui.network"; }

      /// Static function that returns the library name.
      /// Must be implemented for Library registration
      /// @return name of the library
      static std::string library_name() { return "network"; }

      /// Static function that returns the description of the library.
      /// Must be implemented for Library registration
      /// @return description of the library

      static std::string library_description()
      {
        return "This library implements the Network API.";
      }

      /// Gets the Class name
      static std::string type_name() { return "LibNetwork"; }

    }; // end LibNetwork

////////////////////////////////////////////////////////////////////////////////

} // network
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_network_lib_network_hpp
