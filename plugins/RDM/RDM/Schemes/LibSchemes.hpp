// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_LibSchemes_hpp
#define CF_RDM_LibSchemes_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro RDM_SCHEMES_API
#ifdef COOLFLUID_RDM_SCHEMES_EXPORTS
#   define RDM_SCHEMES_API      CF_EXPORT_API
#   define RDM_TEMPLATE
#else
#   define RDM_SCHEMES_API      CF_IMPORT_API
#   define RDM_SCHEMES_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the RDM finite elment method library
/// @author Tiago Quintino
class RDM_SCHEMES_API LibSchemes : public Common::CLibrary {

public:

  typedef boost::shared_ptr<LibSchemes> Ptr;
  typedef boost::shared_ptr<LibSchemes const> ConstPtr;

  /// Constructor
  LibSchemes ( const std::string& name) : Common::CLibrary(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.RDM"; }

  /// Static function that returns the module name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "Schemes"; }

  /// Static function that returns the description of the module.
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

///////////////////////////////////////////////////////////////////////////////////////

/// function returning positive number or zero
inline Real plus ( Real x )
{
  return std::max( 0. , x );
}

/// function returning negative number or zero
inline Real minus ( Real x )
{
  return std::min( 0. , x );
}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_RDM_LibSchemes_hpp
