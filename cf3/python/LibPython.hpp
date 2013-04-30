// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_LibPython_hpp
#define cf3_LibPython_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Python_API
/// @note build system defines COOLFLUID_SOLVER_EXPORTS when compiling MeshTools files
#ifdef COOLFLUID_SOLVER_EXPORTS
#   define Python_API      CF3_EXPORT_API
#   define Python_TEMPLATE
#else
#   define Python_API      CF3_IMPORT_API
#   define Python_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {

  /// Basic Classes for Python wrapper classes used by CF
  namespace python {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library Python
  /// @author Bart Janssens
  class Python_API LibPython : public common::Library {

  public:

    
    

    /// Constructor
    LibPython ( const std::string& name) : common::Library(name) {   }
    
    ~LibPython();

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "cf3.python"; }

    /// Static function that returns the library name.
    /// Must be implemented for Library registration
    /// @return name of the library
    static std::string library_name() { return "python"; }

    /// Static function that returns the description of the library.
    /// Must be implemented for Library registration
    /// @return description of the library

    static std::string library_description()
    {
      return "This library implements the Python API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibPython"; }
    
    virtual void initiate();
    virtual void terminate();

  protected:

    /// initiate library
    virtual void initiate_impl();

    /// terminate library
    virtual void terminate_impl();

  }; // end LibPython

////////////////////////////////////////////////////////////////////////////////

} // python
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_LibPython_hpp
