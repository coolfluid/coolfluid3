// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Physics_Scalar_LibScalar_hpp
#define CF_Physics_Scalar_LibScalar_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro Scalar_API
/// @note build system defines COOLFLUID_PHYSICS_SCALAR_EXPORTS when compiling Scalar files
#ifdef COOLFLUID_PHYSICS_SCALAR_EXPORTS
#   define Scalar_API      CF_EXPORT_API
#   define Scalar_TEMPLATE
#else
#   define Scalar_API      CF_IMPORT_API
#   define Scalar_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Physics {

/// @brief %Scalar transport equations
///
/// @author Tiago Quintino
namespace Scalar {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the Scalar library
/// @author Tiago Quintino
class Scalar_API LibScalar : public Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibScalar> Ptr;
  typedef boost::shared_ptr<LibScalar const> ConstPtr;

  /// Constructor
  LibScalar ( const std::string& name) : Common::CLibrary(name) { }

  virtual ~LibScalar() { }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.Physics.Scalar"; }

  /// Static function that returns the library name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "Scalar"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements Scalar equations";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibScalar"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibScalar

////////////////////////////////////////////////////////////////////////////////

} // Scalar
} // Physics
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Physics_Scalar_LibScalar_hpp

