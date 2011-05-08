// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_SFDM_SF_LibSF_hpp
#define CF_SFDM_SF_LibSF_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro SFDM_SF_API
/// @note build system defines COOLFLUID_SFDM_SF_EXPORTS when compiling SFDM files
#ifdef COOLFLUID_SFDM_SF_EXPORTS
#   define SFDM_SF_API      CF_EXPORT_API
#   define SFDM_SF_TEMPLATE
#else
#   define SFDM_SF_API      CF_IMPORT_API
#   define SFDM_SF_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace SFDM {

/// @brief Shape Function namespace
///
/// This namespace separates all SFDM related shape functions separately
/// @author Willem Deconinck
namespace SF {

////////////////////////////////////////////////////////////////////////////////

/// @brief Defines the %SFDM Shape Function library
/// @author Willem Deconinck
class SFDM_SF_API LibSF :
    public CF::Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibSF> Ptr;
  typedef boost::shared_ptr<LibSF const> ConstPtr;

  /// Constructor
  LibSF ( const std::string& name) : CF::Common::CLibrary(name) { }

  virtual ~LibSF() { }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.SFDM.SF"; }

  /// Static function that returns the module name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "SF"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements ShapeFunction components to use in a Spectral Finite Difference Solver.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibSF"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibSF

////////////////////////////////////////////////////////////////////////////////

} // SF
} // SFDM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_SFDM_SF_LibSF_hpp
