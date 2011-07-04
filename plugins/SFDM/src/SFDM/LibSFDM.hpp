// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_SFDM_LibSFDM_hpp
#define CF_SFDM_LibSFDM_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro SFDM_API
/// @note build system defines COOLFLUID_SFDM_EXPORTS when compiling SFDM files
#ifdef COOLFLUID_SFDM_EXPORTS
#   define SFDM_API      CF_EXPORT_API
#   define SFDM_TEMPLATE
#else
#   define SFDM_API      CF_IMPORT_API
#   define SFDM_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {

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
    public CF::Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibSFDM> Ptr;
  typedef boost::shared_ptr<LibSFDM const> ConstPtr;

  /// Constructor
  LibSFDM ( const std::string& name) : CF::Common::CLibrary(name) { }

  virtual ~LibSFDM() { }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.SFDM"; }

  /// Static function that returns the library name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "SFDM"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements Core components to construct a Spectral Finite Difference Solver.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibSFDM"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibSFDM

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_SFDM_LibSFDM_hpp
