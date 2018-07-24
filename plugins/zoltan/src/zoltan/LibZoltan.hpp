// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_zoltan_LibZoltan_hpp
#define CF_zoltan_LibZoltan_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro ZOLTAN_API
/// @note build system defines COOLFLUID_ZOLTAN_EXPORTS when compiling Zoltan files
#ifdef COOLFLUID_ZOLTAN_EXPORTS
#   define ZOLTAN_API      CF3_EXPORT_API
#   define ZOLTAN_TEMPLATE
#else
#   define ZOLTAN_API      CF3_IMPORT_API
#   define ZOLTAN_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
  
/// @brief Classes for Zoltan operations
namespace zoltan {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the Zoltan operations
/// @author Bart Janssens
class ZOLTAN_API LibZoltan : public cf3::common::Library
{
public:

  
  

  /// Constructor
  LibZoltan ( const std::string& name) : common::Library(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.zoltan"; }

  /// Static function that returns the module name.
  /// Must be implemented for the LibraryRegister template
  /// @return name of the module
  static std::string library_name() { return "zoltan"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for the LibraryRegister template
  /// @return descripton of the module
  static std::string library_description()
  {
    return "This library provides an interface for mesh manipulation using Zoltan.";
  }

  /// Gets the Class name
  static std::string getClassName() { return "LibZoltan"; }

}; // end LibZoltan

////////////////////////////////////////////////////////////////////////////////

} // namespace zoltan
} // namespace cf3

////////////////////////////////////////////////////////////////////////////////

#endif // CF_zoltan_LibZoltan_hpp
