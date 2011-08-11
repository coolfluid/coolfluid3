// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Tools_Growl_LibGrowl_hpp
#define CF_Tools_Growl_LibGrowl_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Tools_Growl_API
/// @note build system defines COOLFLUID_TOOLS_GROWL_EXPORTS when compiling
/// Growl files
#ifdef COOLFLUID_TOOLS_GROWL_EXPORTS
#   define Tools_Growl_API      CF_EXPORT_API
#   define Tools_Growl_TEMPLATE
#else
#   define Tools_Growl_API      CF_IMPORT_API
#   define Tools_Growl_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {

/// @brief Classes for %Growl desktop notifications using the GNTP protocol
///
/// @see Growl::Notifier
/// @author Willem Deconinck
namespace Growl {

////////////////////////////////////////////////////////////////////////////////

/// @brief Defines the initialization and termination of the library %Growl
///
/// @see Growl::Notifier
/// @author Willem Deconinck
class Tools_Growl_API LibGrowl : public Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibGrowl> Ptr;
  typedef boost::shared_ptr<LibGrowl const> ConstPtr;

  /// Constructor
  LibGrowl ( const std::string& name) : Common::CLibrary(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.Tools.Growl"; }

  /// Static function that returns the library name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "Growl"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the Growl notification API.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibGrowl"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibGrowl

////////////////////////////////////////////////////////////////////////////////

} // Growl
} // Tools
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Tools_Growl_LibGrowl_hpp
