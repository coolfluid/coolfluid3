// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Graphics_Core_LibGraphics_hpp
#define CF_GUI_Graphics_Core_LibGraphics_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Graphics_API
/// @note build system defines COOLFLUID_UI_GRAPHICS_EXPORTS when compiling GraphicsTools files
#ifdef COOLFLUID_UI_GRAPHICS_EXPORTS
#   define Graphics_API      CF_EXPORT_API
#   define Graphics_TEMPLATE
#else
#   define Graphics_API      CF_IMPORT_API
#   define Graphics_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace UI {
  /// Basic Classes for Graphics applications used by CF
  namespace Graphics {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library Graphics
  /// @author Tiago Quintino
  class Graphics_API LibGraphics :
      public Common::CLibrary
  {
  public:

    typedef boost::shared_ptr<LibGraphics> Ptr;
    typedef boost::shared_ptr<LibGraphics const> ConstPtr;

    /// Constructor
    LibGraphics ( const std::string& name) : Common::CLibrary(name) {   }

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "CF.GUI.Graphics"; }

    /// Static function that returns the module name.
    /// Must be implemented for CLibrary registration
    /// @return name of the library
    static std::string library_name() { return "Graphics"; }

    /// Static function that returns the description of the module.
    /// Must be implemented for CLibrary registration
    /// @return description of the library

    static std::string library_description()
    {
      return "This library implements the Graphical Interface API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibGraphics"; }

    /// initiate library
    virtual void initiate();

    /// terminate library
    virtual void terminate();

  }; // end LibGraphics

////////////////////////////////////////////////////////////////////////////////

} // Graphics
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Graphics_Core_LibGraphics_hpp
