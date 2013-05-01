// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_LagrangeP0_LibLagrangeP0_hpp
#define cf3_mesh_LagrangeP0_LibLagrangeP0_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"
#include "mesh/LagrangeP0/API.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

/// @brief namespace holding LagrangeP0 shape functions and elements
/// @author Willem Deconinck
namespace LagrangeP0 {

////////////////////////////////////////////////////////////////////////////////

/// Shape functions module for LagrangeP0
/// @author Tiago Quintino, Willem Deconinck
class Mesh_LagrangeP0_API LibLagrangeP0 : public common::Library
{
public:

  
  

  /// Constructor
  LibLagrangeP0 ( const std::string& name) : common::Library(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.mesh.LagrangeP0"; }


  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "LagrangeP0"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the shape functions.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibLagrangeP0"; }
}; // end LibLagrangeP0

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP0
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_LagrangeP0_LibLagrangeP0_hpp
