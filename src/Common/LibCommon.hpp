// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_LibCommon_hpp
#define CF_Common_LibCommon_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hpp"
#include "Common/CommonAPI.hpp"
#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

/// Class defines the initialization and termination of the library Mesh
/// @author Tiago Quintino
class Common_API LibCommon : public Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibCommon> Ptr;
  typedef boost::shared_ptr<LibCommon const> ConstPtr;

  /// Constructor
  LibCommon ( const std::string& name) : Common::CLibrary(name) {}

  /// Configuration options
  static void define_config_properties ( Common::PropertyList& options ) {}

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.Common"; }

  /// Static function that returns the module name.
  /// Must be implemented for CLibrary rCFegistration
  /// @return name of the library
  static std::string library_name() { return "Common"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for CLibrary registration
  /// @return description of the library
  static std::string library_description()
  {
    return "This library implements the Common API.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibCommon"; }

  /// initiate library
  virtual void initiate();

  /// terminate library
  virtual void terminate();

}; // LibCommon

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_LibCommon_hpp
