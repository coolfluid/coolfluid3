// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_LibFVM_hpp
#define CF_FVM_LibFVM_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro FVM_API
/// @note build system defines COOLFLUID_RDM_EXPORTS when compiling FVM files
#ifdef COOLFLUID_FVM_EXPORTS
#   define FVM_API      CF_EXPORT_API
#   define FVM_TEMPLATE
#else
#   define FVM_API      CF_IMPORT_API
#   define FVM_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace FVM {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the finite volume method library
/// @author Willem Deconinck
class FVM_API LibFVM :
    public Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibFVM> Ptr;
  typedef boost::shared_ptr<LibFVM const> ConstPtr;

  /// Constructor
  LibFVM ( const std::string& name) : Common::CLibrary(name) { }

  virtual ~LibFVM() { }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.FVM"; }

  /// Static function that returns the module name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "FVM"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements components to construct a Finite Volume Solver.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibFVM"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibFVM

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_LibFVM_hpp
