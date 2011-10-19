// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_GUI_Graphics_Core_LibGraphics_hpp
#define cf3_GUI_Graphics_Core_LibGraphics_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Graphics_API
/// @note build system defines COOLFLUID_UI_GRAPHICS_EXPORTS when compiling GraphicsTools files
#ifdef COOLFLUID_UI_GRAPHICS_EXPORTS
#   define Graphics_API      CF3_EXPORT_API
#   define Graphics_TEMPLATE
#else
#   define Graphics_API      CF3_IMPORT_API
#   define Graphics_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

  namespace UI {
  /// Basic Classes for Graphics applications used by CF
  namespace Graphics {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library Graphics
  /// @author Tiago Quintino
  class Graphics_API LibGraphics :
      public common::CLibrary
  {
  public:

    typedef boost::shared_ptr<LibGraphics> Ptr;
    typedef boost::shared_ptr<LibGraphics const> ConstPtr;

    /// Constructor
    LibGraphics ( const std::string& name) : common::CLibrary(name) {   }

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "CF.UI.Graphics"; }

    /// Static function that returns the library name.
    /// Must be implemented for CLibrary registration
    /// @return name of the library
    static std::string library_name() { return "Graphics"; }

    /// Static function that returns the description of the library.
    /// Must be implemented for CLibrary registration
    /// @return description of the library

    static std::string library_description()
    {
      return "This library implements the Graphical Interface API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibGraphics"; }

  protected:

    /// initiate library
    virtual void initiate_impl();

    /// terminate library
    virtual void terminate_impl();


  }; // end LibGraphics

////////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // CF3_GUI_Graphics_Core_LibGraphics_hpp
