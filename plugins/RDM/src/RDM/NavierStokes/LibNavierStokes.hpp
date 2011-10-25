// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_LibNavierStokes_hpp
#define cf3_RDM_LibNavierStokes_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro RDM_NAVIERSTOKES_API
/// @note build system defines COOLFLUID_RDM_EXPORTS when compiling
/// RDM files
#ifdef COOLFLUID_RDM_NAVIERSTOKES_EXPORTS
#   define RDM_NAVIERSTOKES_API      CF3_EXPORT_API
#   define RDM_TEMPLATE
#else
#   define RDM_NAVIERSTOKES_API      CF3_IMPORT_API
#   define RDM_NAVIERSTOKES_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the RDM finite elment method library
/// @author Tiago Quintino
class RDM_NAVIERSTOKES_API LibNavierStokes : public common::Library {

public:

  typedef boost::shared_ptr<LibNavierStokes> Ptr;
  typedef boost::shared_ptr<LibNavierStokes const> ConstPtr;

  /// Constructor
  LibNavierStokes ( const std::string& name) : common::Library(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.RDM.NavierStokes"; }

  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "NavierStokes"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements RDM NavierStokes terms.";
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

} // RDM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_RDM_LibNavierStokes_hpp
