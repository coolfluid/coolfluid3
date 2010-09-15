// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_CommonLib_hpp
#define CF_Common_CommonLib_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hpp"
#include "Common/ModuleRegister.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

/// Class defines the initialization and termination of the library Mesh
/// @author Tiago Quintino
class CommonLib : public Common::ModuleRegister<CommonLib>
{
public:

  /// Static function that returns the module name.
  /// Must be implemented for the ModuleRegister template
  /// @return name of the module
  static std::string getModuleName() { return "Common"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for the ModuleRegister template
  /// @return descripton of the module
  static std::string getModuleDescription()
  {
    return "This library implements the Common API.";
  }

  /// Gets the Class name
  static std::string type_name() { return "CommonLib"; }

  /// Start profiling
  virtual void initiate();

  /// Stop profiling
  virtual void terminate();

}; // CommonLib

} // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_CommonLib_hpp
