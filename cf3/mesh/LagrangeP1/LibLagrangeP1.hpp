// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_LagrangeP1_LibLagrangeP1_hpp
#define cf3_mesh_LagrangeP1_LibLagrangeP1_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Library.hpp"
#include "mesh/LagrangeP1/API.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

/// @brief namespace holding LagrangeP1 shape functions and elements
/// @author Willem Deconinck
namespace LagrangeP1 {

////////////////////////////////////////////////////////////////////////////////

/// Shape functions module for LagrangeP1
/// @author Tiago Quintino, Willem Deconinck
class Mesh_LagrangeP1_API LibLagrangeP1 : public common::Library
{
public:

  
  

  /// Constructor
  LibLagrangeP1 ( const std::string& name) : common::Library(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.mesh.LagrangeP1"; }


  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "LagrangeP1"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the shape functions.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibLagrangeP1"; }

}; // end LibLagrangeP1

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP1
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_LagrangeP1_LibLagrangeP1_hpp
