// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UI_ParaView_LIBParaView_HPP
#define CF_UI_ParaView_LIBParaView_HPP

////////////////////////////////////////////////////////////////////////////////

// header
#include "common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro ParaView_API
/// @note build system defines COOLFLUID_UI_ParaView_EXPORTS when compiling ParaViewTools files
#ifdef COOLFLUID_UI_PARAVIEW_EXPORTS
#   define ParaView_API      CF3_EXPORT_API
#   define ParaView_TEMPLATE
#else
#   define ParaView_API      CF3_IMPORT_API
#   define ParaView_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

///////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace ParaView {

///////////////////////////////////////////////////////////////////////////////

class ParaView_API LibParaView :
    public common::CLibrary
{
public:

  typedef boost::shared_ptr<LibParaView> Ptr;
  typedef boost::shared_ptr<LibParaView const> ConstPtr;

  /// Constructor
  LibParaView ( const std::string& name) : common::CLibrary(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.UI.ParaView"; }

  /// Static function that returns the module name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "ParaView"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
      return "This library implements the Paraview Server plugin.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibParaView"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();
};

////////////////////////////////////////////////////////////////////////////////

} // ParaView
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_UI_ParaView_LIBParaView_HPP
