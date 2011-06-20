// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Euler_LibEuler_hpp
#define CF_Euler_LibEuler_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Euler_API
/// @note build system defines COOLFLUID_EULER_EXPORTS when compiling Advection diffusion files
#ifdef COOLFLUID_EULER_EXPORTS
#   define Euler_API      CF_EXPORT_API
#   define TEMPLATE
#else
#   define Euler_API      CF_IMPORT_API
#   define Euler_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {

/// @brief %Physics %Euler classes
///
/// Euler functionality for the %Physics is added in this library
/// @author Willem Deconinck
namespace Euler {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the Euler library
/// @author Willem Deconinck
class Euler_API LibEuler :
    public Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibEuler> Ptr;
  typedef boost::shared_ptr<LibEuler const> ConstPtr;

  /// Constructor
  LibEuler ( const std::string& name) : Common::CLibrary(name) { }

  virtual ~LibEuler() { }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.Euler"; }

  /// Static function that returns the module name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "Euler"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements physics components for Advection Diffusion.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibEuler"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibEuler

////////////////////////////////////////////////////////////////////////////////

} // Euler
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Euler_LibEuler_hpp
