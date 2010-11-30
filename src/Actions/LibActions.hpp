// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_LibActions_hpp
#define CF_LibActions_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

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
    public Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibActions> Ptr;
  typedef boost::shared_ptr<LibActions const> ConstPtr;

  /// Constructor
  LibActions ( const std::string& name) : Common::CLibrary(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.Actions"; }


  /// Static function that returns the module name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "Actions"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the Actions API.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibActions"; }

  /// initiate library
  virtual void initiate();

  /// terminate library
  virtual void terminate();

}; // end LibActions

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_LibActions_hpp
