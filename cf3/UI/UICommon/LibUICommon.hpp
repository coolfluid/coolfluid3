// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_UICommon_LibUICommon_hpp
#define cf3_GUI_UICommon_LibUICommon_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro UICommon_API
/// @note build system defines COOLFLUID_UICommon_EXPORTS when compiling Network files
#ifdef COOLFLUID_UICOMMONs_EXPORTS
#   define UICommon_API CF3_EXPORT_API
#   define UICommon_TEMPLATE
#else
#   define UICommon_API CF3_IMPORT_API
#   define UICommon_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {

/// Common classes for the client and the server
namespace UICommon {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library Network
  /// @author Tiago Quintino
  class UICommon_API LibUICommon :
      public common::Library
  {
  public:

    typedef boost::shared_ptr<LibUICommon> Ptr;
    typedef boost::shared_ptr<LibUICommon const> ConstPtr;

    /// Constructor
    LibUICommon ( const std::string& name) : common::Library(name) {   }

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "CF.UI.UICommon"; }

    /// Static function that returns the library name.
    /// Must be implemented for Library registration
    /// @return name of the library
    static std::string library_name() { return "LibUICommon"; }

    /// Static function that returns the description of the library.
    /// Must be implemented for Library registration
    /// @return description of the library

    static std::string library_description()
    {
      return "This library provides some common code for UI libraries and applications.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibUIConmon"; }

  protected:

    /// initiate library
    virtual void initiate_impl();

    /// terminate library
    virtual void terminate_impl();

  }; // end LibUICommon

////////////////////////////////////////////////////////////////////////////////

} // Network
} // UI
} // cf3

#endif // cf3_GUI_UICommon_LibUICommon_hpp
