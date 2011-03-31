// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_UICommon_LibUICommon_hpp
#define CF_GUI_UICommon_LibUICommon_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro UICommon_API
/// @note build system defines COOLFLUID_UICommon_EXPORTS when compiling Network files
#ifdef COOLFLUID_UICOMMONs_EXPORTS
#   define UICommon_API CF_EXPORT_API
#   define UICommon_TEMPLATE
#else
#   define UICommon_API CF_IMPORT_API
#   define UICommon_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {

/// Common classes for the client and the server
namespace UICommon {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library Network
  /// @author Tiago Quintino
  class UICommon_API LibUICommon :
      public Common::CLibrary
  {
  public:

    typedef boost::shared_ptr<LibUICommon> Ptr;
    typedef boost::shared_ptr<LibUICommon const> ConstPtr;

    /// Constructor
    LibUICommon ( const std::string& name) : Common::CLibrary(name) {   }

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "CF.UI.UICommon"; }

    /// Static function that returns the module name.
    /// Must be implemented for CLibrary registration
    /// @return name of the library
    static std::string library_name() { return "LibUICommon"; }

    /// Static function that returns the description of the module.
    /// Must be implemented for CLibrary registration
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
} // CF

#endif // CF_GUI_UICommon_LibUICommon_hpp
