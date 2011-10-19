// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_LibGPU_hpp
#define CF_RDM_LibGPU_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/CLibrary.hpp"

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
class RDM_GPU_API LibGPU : public Common::CLibrary {

public:

  typedef boost::shared_ptr<LibGPU> Ptr;
  typedef boost::shared_ptr<LibGPU const> ConstPtr;

  /// Constructor
  LibGPU ( const std::string& name) : Common::CLibrary(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.RDM.GPU"; }

  /// Static function that returns the module name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "GPU"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements RDM GPU terms.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibGPU"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibGPU

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_LibGPU_hpp
