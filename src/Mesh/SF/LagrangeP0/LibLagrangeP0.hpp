// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_SF_LagrangeP0_LibLagrangeP0_hpp
#define CF_Mesh_SF_LagrangeP0_LibLagrangeP0_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro SF_API
/// @note build system defines COOLFLUID_MESH_SF_LAGRANGEP0_EXPORTS when compiling SF files
#ifdef COOLFLUID_MESH_SF_EXPORTS
#   define Mesh_SF_LagrangeP0_API      CF_EXPORT_API
#   define Mesh_SF_LagrangeP0_TEMPLATE
#else
#   define Mesh_SF_LagrangeP0_API      CF_IMPORT_API
#   define Mesh_SF_LagrangeP0_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace SF {

/// @brief namespace holding LagrangeP0 shape functions
/// @author Willem Deconinck
namespace LagrangeP0 {

////////////////////////////////////////////////////////////////////////////////

/// Shape functions module for LagrangeP0
/// @author Tiago Quintino, Willem Deconinck, Bart Janssens
class Mesh_SF_LagrangeP0_API LibLagrangeP0 : public Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibLagrangeP0> Ptr;
  typedef boost::shared_ptr<LibLagrangeP0 const> ConstPtr;

  /// Constructor
  LibLagrangeP0 ( const std::string& name) : Common::CLibrary(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.Mesh.SF.LagrangeP0"; }


  /// Static function that returns the library name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "LagrangeP0"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the shape functions.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibLagrangeP0"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibLagrangeP0

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP0
} // SF
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_SF_LagrangeP0_LibLagrangeP0_hpp
