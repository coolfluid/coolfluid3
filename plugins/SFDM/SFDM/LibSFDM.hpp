// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_LibSFDM_hpp
#define cf3_SFDM_LibSFDM_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro SFDM_API
/// @note build system defines COOLFLUID_SFDM_EXPORTS when compiling SFDM files
#ifdef COOLFLUID_SFDM_EXPORTS
#   define SFDM_API      CF3_EXPORT_API
#   define SFDM_TEMPLATE
#else
#   define SFDM_API      CF3_IMPORT_API
#   define SFDM_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

/// @brief Spectral Finite Difference Method namespace
///
/// The Spectral Finite Difference Method is a high-order method
/// for solving systems of partial differential equations.
/// @author Willem Deconinck
namespace SFDM {

////////////////////////////////////////////////////////////////////////////////

/// @brief Defines the Spectral Finite Difference Core library
///
/// This library implements Core components to construct a Spectral Finite Difference Solver.";
/// @author Willem Deconinck
class SFDM_API LibSFDM :
    public cf3::common::Library
{
public:

  typedef boost::shared_ptr<LibSFDM> Ptr;
  typedef boost::shared_ptr<LibSFDM const> ConstPtr;

  /// Constructor
  LibSFDM ( const std::string& name) : cf3::common::Library(name) { }

  virtual ~LibSFDM() { }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.SFDM"; }

  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "SFDM"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements Core components to construct a Spectral Finite Difference Solver.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibSFDM"; }
}; // end LibSFDM

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_SFDM_LibSFDM_hpp
