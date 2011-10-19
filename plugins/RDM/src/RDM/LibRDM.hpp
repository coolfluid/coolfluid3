// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_LibRDM_hpp
#define cf3_RDM_LibRDM_hpp

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/// Define the macro RDM_API
/// @note build system defines COOLFLUID_RDM_EXPORTS when compiling
/// RDM files
#ifdef COOLFLUID_RDM_CORE_EXPORTS
#   define RDM_API      CF3_EXPORT_API
#   define RDM_TEMPLATE
#else
#   define RDM_API      CF3_IMPORT_API
#   define RDM_CORE_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////////////////

/// Class defines the RDM finite elment method library
/// @author Tiago Quintino
class RDM_API LibRDM : public common::CLibrary {

public:

  typedef boost::shared_ptr<LibRDM> Ptr;
  typedef boost::shared_ptr<LibRDM const> ConstPtr;

  /// Constructor
  LibRDM ( const std::string& name) : common::CLibrary(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.RDM"; }

  /// Static function that returns the library name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "RDM"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements a Residual Distribution Solver.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibRDM"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibRDM

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3

#endif // CF3_RDM_LibRDM_hpp
