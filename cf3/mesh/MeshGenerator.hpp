// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_MeshGenerator_hpp
#define cf3_mesh_MeshGenerator_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Action.hpp"

#include "mesh/LibMesh.hpp"

namespace cf3 {
namespace mesh {

  class Mesh;

////////////////////////////////////////////////////////////////////////////////

/// MeshGenerator component class
/// This class serves as a component that that will read
/// the mesh format from file
/// @author Willem Deconinck
class Mesh_API MeshGenerator : public common::Action {

public: // typedefs

  /// type of pointer to Component
  
  /// type of pointer to constant Component
  

public: // functions

  /// Contructor
  /// @param name of the component
  MeshGenerator ( const std::string& name );

  /// Virtual destructor
  virtual ~MeshGenerator();

  /// Get the class name
  static std::string type_name () { return "MeshGenerator"; }

  /// execute
  virtual void execute() = 0;

  /// generate, wraps execute() and returns the mesh reference
  Mesh& generate();

private: // functions

  void config_mesh();

protected: // data

  Handle<Mesh> m_mesh;

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_MeshGenerator_hpp
