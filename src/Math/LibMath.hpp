// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_MathAPI_hpp
#define CF_MathAPI_hpp

////////////////////////////////////////////////////////////////////////////////

#include <cmath>  // all sorts of mathematical functions and constants

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Math_API
/// @note build system defines COOLFLUID_MATH_EXPORTS when compiling MathTools files
#ifdef COOLFLUID_MATH_EXPORTS
#   define Math_API      CF_EXPORT_API
#   define Math_TEMPLATE
#else
#   define Math_API      CF_IMPORT_API
#   define Math_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {

  /// Basic Classes for Mathematical applications used by the CF
  namespace Math {

////////////////////////////////////////////////////////////////////////////////

    /// Class defines the initialization and termination of the library MeshDiff
    /// @author Tiago Quintino
    class Math_API LibMath :  public Common::CLibrary
    {
    public:

      typedef boost::shared_ptr<LibMath> Ptr;
      typedef boost::shared_ptr<LibMath const> ConstPtr;

      /// Constructor
      LibMath ( const std::string& name) : Common::CLibrary(name) {   }

    public: // functions

      /// @return string of the library namespace
      static std::string library_namespace() { return "CF.Math"; }

      /// Static function that returns the module name.
      /// Must be implemented for CLibrary registration
      /// @return name of the library
      static std::string library_name() { return "Math"; }

      /// Static function that returns the description of the module.
      /// Must be implemented for CLibrary registration
      /// @return description of the library

      static std::string library_description()
      {
        return "This library implements the MeshDiff manipulation API.";
      }

      /// Gets the Class name
      static std::string type_name() { return "LibMath"; }

    protected:

      /// initiate library
      virtual void initiate_impl();

      /// terminate library
      virtual void terminate_impl();

    }; // end LibMath

////////////////////////////////////////////////////////////////////////////////

} // Math
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_MathAPI_hpp
