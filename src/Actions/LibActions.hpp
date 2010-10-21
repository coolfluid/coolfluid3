// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_LibActions_hpp
#define CF_LibActions_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/ExportAPI.hpp"
#include "Common/LibraryRegister.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Actions_API
/// @note build system defines COOLFLUID_ACTIONS_EXPORTS when compiling MeshTools files
#ifdef COOLFLUID_ACTIONS_EXPORTS
#   define Actions_API      CF_EXPORT_API
#   define Actions_TEMPLATE
#else
#   define Actions_API      CF_IMPORT_API
#   define Actions_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  /// Basic Classes for Actions used by CF
  namespace Actions {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the initialization and termination of the library Actions
class Actions_API LibActions :
    public Common::LibraryRegister<LibActions>
{
public:

  /// Static function that returns the module name.
  /// Must be implemented for the LibraryRegister template
  /// @return name of the module
  static std::string library_name() { return "Actions"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for the LibraryRegister template
  /// @return descripton of the module
  static std::string library_description()
  {
    return "This library implements the Actions API.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibActions"; }

  /// Start profiling
  virtual void initiate();

  /// Stop profiling
  virtual void terminate();

}; // end LibActions

////////////////////////////////////////////////////////////////////////////////

} // namespace Actions
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_LibActions_hpp
