// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_LibTecplot_hpp
#define CF_LibTecplot_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Tecplot_API
/// @note build system defines COOLFLUID_GMSH_EXPORTS when compiling Tecplot files
#ifdef COOLFLUID_GMSH_EXPORTS
#   define Tecplot_API      CF_EXPORT_API
#   define Tecplot_TEMPLATE
#else
#   define Tecplot_API      CF_IMPORT_API
#   define Tecplot_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  
/// @brief Library for I/O of the Tecplot format 
namespace Tecplot {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the Tecplottral mesh format operations
/// @author Willem Deconinck
class Tecplot_API LibTecplot :
    public Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibTecplot> Ptr;
  typedef boost::shared_ptr<LibTecplot const> ConstPtr;

  /// Constructor
  LibTecplot ( const std::string& name) : Common::CLibrary(name) {   }

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.Mesh.Tecplot"; }

  /// Static function that returns the module name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "Tecplot"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the Tecplot mesh format operations.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibTecplot"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibTecplot

////////////////////////////////////////////////////////////////////////////////

} // Tecplot
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_LibTecplot_hpp
