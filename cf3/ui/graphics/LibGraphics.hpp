// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_Graphics_core_LibGraphics_hpp
#define cf3_ui_Graphics_core_LibGraphics_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Graphics_API
/// @note build system defines COOLFLuiD_ui_GRAPHICS_EXPORTS when compiling GraphicsTools files
#ifdef COOLFLuiD_ui_GRAPHICS_EXPORTS
#   define Graphics_API      CF3_EXPORT_API
#   define Graphics_TEMPLATE
#else
#   define Graphics_API      CF3_IMPORT_API
#   define Graphics_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

  namespace ui {
  /// Basic Classes for Graphics applications used by CF
  namespace graphics {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library Graphics
  /// @author Tiago QUintino
  class Graphics_API LibGraphics :
      public common::Library
  {
  public:

    typedef boost::shared_ptr<LibGraphics> Ptr;
    typedef boost::shared_ptr<LibGraphics const> ConstPtr;

    /// Constructor
    LibGraphics ( const std::string& name) : common::Library(name) {   }

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "cf3.ui.Graphics"; }

    /// Static function that returns the library name.
    /// Must be implemented for Library registration
    /// @return name of the library
    static std::string library_name() { return "Graphics"; }

    /// Static function that returns the description of the library.
    /// Must be implemented for Library registration
    /// @return description of the library

    static std::string library_description()
    {
      return "This library implements the Graphical Interface API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibGraphics"; }


  }; // end LibGraphics

////////////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_Graphics_core_LibGraphics_hpp
