// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Neu_LibNeu_hpp
#define cf3_mesh_Neu_LibNeu_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Neu_API
/// @note build system defines COOLFLUID_NEU_EXPORTS when compiling Neu files
#ifdef COOLFLUID_NEU_EXPORTS
#   define Neu_API      CF3_EXPORT_API
#   define Neu_TEMPLATE
#else
#   define Neu_API      CF3_IMPORT_API
#   define Neu_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
  
/// @brief Library for I/O of the neutral format
namespace Neu {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the Neutral mesh format operations
/// @author Willem Deconinck
class Neu_API LibNeu : public common::Library
{
public:

  typedef boost::shared_ptr<LibNeu> Ptr;
  typedef boost::shared_ptr<LibNeu const> ConstPtr;

  /// Constructor
  LibNeu ( const std::string& name) : common::Library(name) {   }

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.mesh.Neu"; }

  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "Neu"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the Neutral mesh format operations.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibNeu"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // LibNeu

////////////////////////////////////////////////////////////////////////////////

} // Neu
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Neu_LibNeu_hpp
