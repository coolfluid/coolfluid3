// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_LibUFEM_hpp
#define cf3_UFEM_LibUFEM_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro UFEM_API
/// @note build system defines COOLFLUID_BLOCKMESH_READER_EXPORTS when compiling
/// UFEM files
#ifdef COOLFLUID_BLOCKMESH_READER_EXPORTS
#   define UFEM_API      CF3_EXPORT_API
#   define UFEM_TEMPLATE
#else
#   define UFEM_API      CF3_IMPORT_API
#   define UFEM_TEMPLATE CF3_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UFEM {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the UFEM finite elment method library
/// @author Bart Janssens
class UFEM_API LibUFEM :
    public common::CLibrary
{
public:

  typedef boost::shared_ptr<LibUFEM> Ptr;
  typedef boost::shared_ptr<LibUFEM const> ConstPtr;

  /// Constructor
  LibUFEM ( const std::string& name) : common::CLibrary(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.UFEM"; }

  /// Static function that returns the library name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "UFEM"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements some UFEM compatibility functions.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibUFEM"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibUFEM

////////////////////////////////////////////////////////////////////////////////

} // UFEM
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_UFEM_LibUFEM_hpp
