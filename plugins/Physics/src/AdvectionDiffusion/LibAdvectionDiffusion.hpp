// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_AdvectionDiffusion_LibAdvectionDiffusion_hpp
#define CF_AdvectionDiffusion_LibAdvectionDiffusion_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro AdvectionDiffusion_API
/// @note build system defines COOLFLUID_ADVECTIONDIFFUSION_EXPORTS when compiling Advection diffusion files
#ifdef COOLFLUID_AdvectionDiffusion_EXPORTS
#   define AdvectionDiffusion_API      CF_EXPORT_API
#   define TEMPLATE
#else
#   define AdvectionDiffusion_API      CF_IMPORT_API
#   define AdvectionDiffusion_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {

/// @brief %Physics %AdvectionDiffusion classes
///
/// AdvectionDiffusion functionality for the %Physics is added in this library
/// @author Willem Deconinck
namespace AdvectionDiffusion {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the advection-diffusion library
/// @author Willem Deconinck
class AdvectionDiffusion_API LibAdvectionDiffusion :
    public Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibAdvectionDiffusion> Ptr;
  typedef boost::shared_ptr<LibAdvectionDiffusion const> ConstPtr;

  /// Constructor
  LibAdvectionDiffusion ( const std::string& name) : Common::CLibrary(name) { }

  virtual ~LibAdvectionDiffusion() { }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.AdvectionDiffusion"; }

  /// Static function that returns the module name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "AdvectionDiffusion"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements physics components for Advection Diffusion.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibAdvectionDiffusion"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibAdvectionDiffusion

////////////////////////////////////////////////////////////////////////////////

} // AdvectionDiffusion
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_AdvectionDiffusion_LibAdvectionDiffusion_hpp
