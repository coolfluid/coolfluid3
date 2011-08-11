// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RungeKutta_LibRungeKutta_hpp
#define CF_RungeKutta_LibRungeKutta_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro RungeKutta_API
/// @note build system defines COOLFLUID_RUNGEKUTTA_EXPORTS when compiling RungeKutta files
#ifdef COOLFLUID_RUNGEKUTTA_EXPORTS
#   define RungeKutta_API      CF_EXPORT_API
#   define RungeKutta_TEMPLATE
#else
#   define RungeKutta_API      CF_IMPORT_API
#   define RungeKutta_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
/// @brief Runge Kutta differential equation solver classes
///
/// Solves differential equations using the Runge-Kutta multistage method.
/// @author Willem Deconinck
namespace RungeKutta {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the RungeKutta library
/// @author Willem Deconinck
class RungeKutta_API LibRungeKutta :
    public Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibRungeKutta> Ptr;
  typedef boost::shared_ptr<LibRungeKutta const> ConstPtr;

  /// Constructor
  LibRungeKutta ( const std::string& name) : Common::CLibrary(name) { }

  virtual ~LibRungeKutta() { }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.RungeKutta"; }

  /// Static function that returns the library name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "RungeKutta"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements components to construct a Runge-Kutta differential equation solver.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibRungeKutta"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibRungeKutta

////////////////////////////////////////////////////////////////////////////////

} // RungeKutta
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_RungeKuttaLibRungeKutta_hpp
