// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_lineuler_LibLinEuler_hpp
#define cf3_SFDM_lineuler_LibLinEuler_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro lineuler_API
/// @note build system defines COOLFLUID_SFDM_LINEULER_EXPORTS when compiling lineuler files
#ifdef COOLFLUID_SFDM_LINEULER_EXPORTS
#   define SFDM_lineuler_API      CF3_EXPORT_API
#   define SFDM_lineuler_TEMPLATE
#else
#   define SFDM_lineuler_API      CF3_IMPORT_API
#   define SFDM_lineuler_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {
namespace lineuler {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the lineuler library
class SFDM_lineuler_API LibLinEuler : public common::Library
{
public:

  
  

  /// Constructor
  LibLinEuler ( const std::string& name) : common::Library(name) { }

  virtual ~LibLinEuler() { }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.SFDM.lineuler"; }

  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "lineuler"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements lineuler equations";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibLinEuler"; }

}; // end LibLinEuler

////////////////////////////////////////////////////////////////////////////////

} // lineuler
} // SFDM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_SFDM_lineuler_LibLinEuler_hpp

