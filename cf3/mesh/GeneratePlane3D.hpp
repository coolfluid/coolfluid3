// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_GeneratePlane3D_hpp
#define cf3_mesh_GeneratePlane3D_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Action.hpp"

#include "mesh/MeshGenerator.hpp"
#include "mesh/LibMesh.hpp"

namespace cf3 {
namespace mesh {

  class Mesh;

////////////////////////////////////////////////////////////////////////////////

/// GeneratePlane3D component class
/// Create a plane in 3D
class Mesh_API GeneratePlane3D : public MeshGenerator {

public: // functions

  /// Contructor
  /// @param name of the component
  GeneratePlane3D ( const std::string& name );

  /// Virtual destructor
  virtual ~GeneratePlane3D();

  /// Get the class name
  static std::string type_name () { return "GeneratePlane3D"; }
  
  virtual void execute();

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_GeneratePlane3D_hpp
