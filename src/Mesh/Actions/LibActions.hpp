// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_Actions_LibActions_hpp
#define CF_Mesh_Actions_LibActions_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Actions_API
/// @note build system defines COOLFLUID_ACTIONS_EXPORTS when compiling MeshTools files
#ifdef COOLFLUID_MESH_ACTIONS_EXPORTS
#   define Mesh_Actions_API      CF_EXPORT_API
#   define Mesh_Actions_TEMPLATE
#else
#   define Mesh_Actions_API      CF_IMPORT_API
#   define Mesh_Actions_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh{

/// @brief CAction derived classes for mesh manipulations
namespace Actions {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the initialization and termination of the library Actions
class Mesh_Actions_API LibActions :
    public Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibActions> Ptr;
  typedef boost::shared_ptr<LibActions const> ConstPtr;

  /// Constructor
  LibActions ( const std::string& name) : Common::CLibrary(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.Mesh.Actions"; }


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

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibActions

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Actions_LibActions_hpp
