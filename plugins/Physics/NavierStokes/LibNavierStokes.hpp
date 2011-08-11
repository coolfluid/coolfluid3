// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_NavierStokes_LibNavierStokes_hpp
#define CF_NavierStokes_LibNavierStokes_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro NavierStokes_API
/// @note build system defines COOLFLUID_PHYSICS_NAVIERSTOKES_EXPORTS when compiling Advection diffusion files
#ifdef COOLFLUID_PHYSICS_NAVIERSTOKES_EXPORTS
#   define NavierStokes_API      CF_EXPORT_API
#   define TEMPLATE
#else
#   define NavierStokes_API      CF_IMPORT_API
#   define NavierStokes_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Physics {

/// @brief %Physics %NavierStokes classes
///
/// NavierStokes functionality for the %Physics is added in this library
/// @author Willem Deconinck
namespace NavierStokes {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the NavierStokes library
/// @author Tiago Quintino
class NavierStokes_API LibNavierStokes : public Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibNavierStokes> Ptr;
  typedef boost::shared_ptr<LibNavierStokes const> ConstPtr;

  /// Constructor
  LibNavierStokes ( const std::string& name) : Common::CLibrary(name) { }

  virtual ~LibNavierStokes() { }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.Physics.NavierStokes"; }

  /// Static function that returns the library name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "NavierStokes"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements physics components for the Navier-Stokes fluid flow equations.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibNavierStokes"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibNavierStokes

////////////////////////////////////////////////////////////////////////////////

} // NavierStokes
} // Physics
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_NavierStokes_LibNavierStokes_hpp
