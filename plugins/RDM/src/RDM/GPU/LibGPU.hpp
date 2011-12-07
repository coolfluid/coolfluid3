// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_LibGPU_hpp
#define CF_RDM_LibGPU_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro RDM_GPU_API
/// @note build system defines COOLFLUID_RDM_EXPORTS when compiling
/// RDM files
#ifdef COOLFLUID_RDM_GPU_EXPORTS
#   define RDM_GPU_API      CF_EXPORT_API
#   define RDM_TEMPLATE
#else
#   define RDM_GPU_API      CF_IMPORT_API
#   define RDM_GPU_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the RDM finite elment method library
/// @author Tiago Quintino
class RDM_GPU_API LibGPU : public common::Library {

public:

  
  

  /// Constructor
  LibGPU ( const std::string& name) : common::Library(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.RDM.GPU"; }

  /// Static function that returns the module name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "GPU"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements RDM GPU terms.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibGPU"; }

}; // end LibGPU

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_LibGPU_hpp
