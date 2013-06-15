// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_LibCommon_hpp
#define cf3_common_LibCommon_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/CommonAPI.hpp"
#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// COOLFluiD Classes
namespace cf3 {

/// Common Classes for Component Environment
namespace common {

/// Class defines the initialization and termination of the library mesh
/// @author Tiago Quintino
class Common_API LibCommon : public common::Library
{
public:

  /// Constructor
  LibCommon ( const std::string& name) : common::Library(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.common"; }

  /// Static function that returns the library name.
  /// Must be implemented for Library rCFegistration
  /// @return name of the library
  static std::string library_name() { return "common"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library
  static std::string library_description()
  {
    return "This library implements the common API.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibCommon"; }
}; // LibCommon

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_LibCommon_hpp
