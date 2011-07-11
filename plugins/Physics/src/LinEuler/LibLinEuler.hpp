// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_LinEuler_LibLinEuler_hpp
#define CF_LinEuler_LibLinEuler_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro LinEuler_API
/// @note build system defines COOLFLUID_LINEULER_EXPORTS when compiling LinEuler files
#ifdef COOLFLUID_LINEULER_EXPORTS
#   define LinEuler_API      CF_EXPORT_API
#   define TEMPLATE
#else
#   define LinEuler_API      CF_IMPORT_API
#   define LinEuler_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {

/// @brief %LinEuler classes
///
/// LinEuler library
/// @author 
namespace LinEuler {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the LinEuler library
/// @author 
class LinEuler_API LibLinEuler : public Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibLinEuler> Ptr;
  typedef boost::shared_ptr<LibLinEuler const> ConstPtr;

  /// Constructor
  LibLinEuler ( const std::string& name) : Common::CLibrary(name) { }

  virtual ~LibLinEuler() { }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.LinEuler"; }

  /// Static function that returns the library name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "LinEuler"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements LinEuler";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibLinEuler"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibLinEuler

////////////////////////////////////////////////////////////////////////////////

} // LinEuler
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_LinEuler_LibLinEuler_hpp

