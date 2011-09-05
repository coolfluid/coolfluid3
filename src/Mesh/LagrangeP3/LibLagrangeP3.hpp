// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_LagrangeP3_LibLagrangeP3_hpp
#define CF_Mesh_LagrangeP3_LibLagrangeP3_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro SF_API
/// @note build system defines COOLFLUID_MESH_LagrangeP3_EXPORTS when compiling SF files
#ifdef COOLFLUID_MESH_LagrangeP3_EXPORTS
#   define Mesh_LagrangeP3_API      CF_EXPORT_API
#   define Mesh_LagrangeP3_TEMPLATE
#else
#   define Mesh_LagrangeP3_API      CF_IMPORT_API
#   define Mesh_LagrangeP3_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

/// @brief namespace holding LagrangeP3 shape functions and elements
/// @author Willem Deconinck
namespace LagrangeP3 {

////////////////////////////////////////////////////////////////////////////////

/// Shape functions module for LagrangeP3
/// @author Tiago Quintino, Willem Deconinck
class Mesh_LagrangeP3_API LibLagrangeP3 : public Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibLagrangeP3> Ptr;
  typedef boost::shared_ptr<LibLagrangeP3 const> ConstPtr;

  /// Constructor
  LibLagrangeP3 ( const std::string& name) : Common::CLibrary(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.Mesh.LagrangeP3"; }


  /// Static function that returns the library name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "LagrangeP3"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the shape functions.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibLagrangeP3"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibLagrangeP3

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP3
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_LagrangeP3_LibLagrangeP3_hpp
