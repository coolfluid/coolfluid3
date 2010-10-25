// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_LibFParser_hpp
#define CF_LibFParser_hpp

////////////////////////////////////////////////////////////////////////////////

#include <string>

#include "../../src/Common/ExportAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro FParser_API
/// @note build system defines FParser_EXPORTS when compiling FParser files
#ifdef FParser_EXPORTS
#   define FParser_API      CF_EXPORT_API
#   define FParser_TEMPLATE
#else
#   define FParser_API      CF_IMPORT_API
#   define FParser_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  namespace FParser {

////////////////////////////////////////////////////////////////////////////////

  /// The purpose of this class is to force Windows compiler to generate a lib
  /// file for this library
  /// @author Tiago Quintino
  /// @author Quentin Gasper
  class FParser_API LibFParser 
  {
  public:

    /// Static function that returns the module name.
    /// Must be implemented for the LibraryRegister template
    /// @return name of the module
    static std::string library_name() { return "FParser"; }

    /// Static function that returns the description of the module.
    /// Must be implemented for the LibraryRegister template
    /// @return descripton of the module
    static std::string library_description()
    {
      return "This library implements the FParser API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibFParser"; }

    /// Start profiling
    virtual void initiate();

    /// Stop profiling
    virtual void terminate();

  }; // end LibFParser

////////////////////////////////////////////////////////////////////////////////

} // namespace FParser
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_LibFParser_hpp
