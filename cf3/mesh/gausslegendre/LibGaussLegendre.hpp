// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_gausslegendre_LibGaussLegendre_hpp
#define cf3_mesh_gausslegendre_LibGaussLegendre_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"
#include "mesh/gausslegendre/API.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

/// @brief namespace holding GaussLegendre quadrature
/// @author Willem Deconinck
namespace gausslegendre {

////////////////////////////////////////////////////////////////////////////////

/// Quadrature module for GaussLegendre
/// @author Willem Deconinck
class mesh_gausslegendre_API LibGaussLegendre : public common::Library
{
public:

  /// Constructor
  LibGaussLegendre ( const std::string& name) : common::Library(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.mesh.gausslegendre"; }


  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "GaussLegendre"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the shape functions.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibGaussLegendre"; }

}; // end LibGaussLegendre

////////////////////////////////////////////////////////////////////////////////

} // gausslegendre
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_GaussLegendre_LibGaussLegendre_hpp
