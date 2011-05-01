// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_SFDM_Core_LibCore_hpp
#define CF_SFDM_Core_LibCore_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro SFDM_Core_API
/// @note build system defines COOLFLUID_RDM_EXPORTS when compiling FVM files
#ifdef COOLFLUID_SFDM_Core_EXPORTS
#   define SFDM_Core_API      CF_EXPORT_API
#   define SFDM_Core_TEMPLATE
#else
#   define SFDM_Core_API      CF_IMPORT_API
#   define SFDM_Core_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace SFDM {
namespace Core {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the Spectral Finite Difference Core library
/// @author Willem Deconinck
class SFDM_Core_API LibCore :
    public CF::Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibCore> Ptr;
  typedef boost::shared_ptr<LibCore const> ConstPtr;

  /// Constructor
  LibCore ( const std::string& name) : CF::Common::CLibrary(name) { }

  virtual ~LibCore() { }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.SFDM.Core"; }

  /// Static function that returns the module name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "Core"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements Core components to construct a Spectral Finite Difference Solver.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibCore"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibCore

////////////////////////////////////////////////////////////////////////////////

} // Core
} // SFDM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_SFDM_Core_LibCore_hpp
