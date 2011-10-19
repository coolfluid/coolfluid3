// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_RDM_LibSchemes_hpp
#define cf3_RDM_LibSchemes_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro RDM_SCHEMES_API
#ifdef COOLFLUID_RDM_SCHEMES_EXPORTS
#   define RDM_SCHEMES_API      CF3_EXPORT_API
#   define RDM_TEMPLATE
#else
#   define RDM_SCHEMES_API      CF3_IMPORT_API
#   define RDM_SCHEMES_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the RDM finite elment method library
/// @author Tiago Quintino
class RDM_SCHEMES_API LibSchemes : public common::CLibrary {

public:

  typedef boost::shared_ptr<LibSchemes> Ptr;
  typedef boost::shared_ptr<LibSchemes const> ConstPtr;

  /// Constructor
  LibSchemes ( const std::string& name) : common::CLibrary(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.RDM.Schemes"; }

  /// Static function that returns the library name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "Schemes"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements basic RDM schemes.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibSchemes"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibSchemes

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_RDM_LibSchemes_hpp
