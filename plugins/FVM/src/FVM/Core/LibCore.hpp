// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_Core_LibCore_hpp
#define CF_FVM_Core_LibCore_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro FVM_Core_API
/// @note build system defines COOLFLUID_RDM_EXPORTS when compiling FVM files
#ifdef COOLFLUID_FVM_Core_EXPORTS
#   define FVM_Core_API      CF_EXPORT_API
#   define FVM_TEMPLATE
#else
#   define FVM_Core_API      CF_IMPORT_API
#   define FVM_Core_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace FVM {
namespace Core {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the finite volume method library
/// @author Willem Deconinck
class FVM_Core_API LibCore :
    public Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibCore> Ptr;
  typedef boost::shared_ptr<LibCore const> ConstPtr;

  /// Constructor
  LibCore ( const std::string& name) : Common::CLibrary(name) { }

  virtual ~LibCore() { }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.FVM.Core"; }

  /// Static function that returns the module name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "Core"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements components to construct a Finite Volume Solver.";
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
} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_CoreLibCore_hpp
