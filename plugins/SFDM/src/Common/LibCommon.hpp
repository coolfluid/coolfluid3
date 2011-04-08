// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_SFDM_Common_LibCommon_hpp
#define CF_SFDM_Common_LibCommon_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro SFDM_Common_API
/// @note build system defines COOLFLUID_RDM_EXPORTS when compiling FVM files
#ifdef COOLFLUID_SFDM_Common_EXPORTS
#   define SFDM_Common_API      CF_EXPORT_API
#   define SFDM_Common_TEMPLATE
#else
#   define SFDM_Common_API      CF_IMPORT_API
#   define SFDM_Common_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace SFDM {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the Spectral Finite Difference Common library
/// @author Willem Deconinck
class SFDM_Common_API LibCommon :
    public CF::Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibCommon> Ptr;
  typedef boost::shared_ptr<LibCommon const> ConstPtr;

  /// Constructor
  LibCommon ( const std::string& name) : CF::Common::CLibrary(name) { }

  virtual ~LibCommon() { }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.SFDM.Common"; }

  /// Static function that returns the module name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "FVM"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements Common components to construct a Spectral Finite Difference Solver.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibCommon"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibCommon

////////////////////////////////////////////////////////////////////////////////

} // Common
} // SFDM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_SFDM_Common_LibCommon_hpp
