// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_LibCGAL_hpp
#define CF_LibCGAL_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro CGAL_API
/// @note build system defines COOLFLUID_CGAL_EXPORTS when compiling CGAL files
#ifdef COOLFLUID_CGAL_EXPORTS
#   define CGAL_API      CF_EXPORT_API
#   define CGAL_TEMPLATE
#else
#   define CGAL_API      CF_IMPORT_API
#   define CGAL_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace Mesh {
  
/// @brief Classes for CGAL mesh format operations
namespace CGAL {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the CGAL mesh format operations
/// @author Bart Janssens
class CGAL_API LibCGAL : public cf3::common::CLibrary
{
public:

  typedef boost::shared_ptr<LibCGAL> Ptr;
  typedef boost::shared_ptr<LibCGAL const> ConstPtr;

  /// Constructor
  LibCGAL ( const std::string& name) : common::CLibrary(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.Mesh.CGAL"; }

  /// Static function that returns the module name.
  /// Must be implemented for the LibraryRegister template
  /// @return name of the module
  static std::string library_name() { return "CGAL"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for the LibraryRegister template
  /// @return descripton of the module
  static std::string library_description()
  {
    return "This library provides an interface for the CGAL 3D tetrahedral mesher.";
  }

  /// Gets the Class name
  static std::string getClassName() { return "LibCGAL"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end CGALLib

////////////////////////////////////////////////////////////////////////////////

} // namespace CGAL
} // namespace Mesh
} // namespace cf3

////////////////////////////////////////////////////////////////////////////////

#endif // CF_LibCGAL_hpp
