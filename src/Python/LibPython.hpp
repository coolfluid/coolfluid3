// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_LibPython_hpp
#define CF_LibPython_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Python_API
/// @note build system defines COOLFLUID_SOLVER_EXPORTS when compiling MeshTools files
#ifdef COOLFLUID_SOLVER_EXPORTS
#   define Python_API      CF_EXPORT_API
#   define Python_TEMPLATE
#else
#   define Python_API      CF_IMPORT_API
#   define Python_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  /// Basic Classes for Python wrapper classes used by CF
  namespace Python {

////////////////////////////////////////////////////////////////////////////////

  /// Class defines the initialization and termination of the library Python
  /// @author Bart Janssens
  class Python_API LibPython : public Common::CLibrary {

  public:

    typedef boost::shared_ptr<LibPython> Ptr;
    typedef boost::shared_ptr<LibPython const> ConstPtr;

    /// Constructor
    LibPython ( const std::string& name) : Common::CLibrary(name) {   }

  public: // functions

    /// @return string of the library namespace
    static std::string library_namespace() { return "CF.Python"; }

    /// Static function that returns the library name.
    /// Must be implemented for CLibrary registration
    /// @return name of the library
    static std::string library_name() { return "Python"; }

    /// Static function that returns the description of the library.
    /// Must be implemented for CLibrary registration
    /// @return description of the library

    static std::string library_description()
    {
      return "This library implements the Python API.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibPython"; }

  protected:

    /// initiate library
    virtual void initiate_impl();

    /// terminate library
    virtual void terminate_impl();

  }; // end LibPython

////////////////////////////////////////////////////////////////////////////////

} // Python
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_LibPython_hpp
