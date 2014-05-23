// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_GenerateLine3D_hpp
#define cf3_mesh_GenerateLine3D_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Action.hpp"

#include "mesh/MeshGenerator.hpp"
#include "mesh/LibMesh.hpp"

namespace cf3 {
namespace mesh {

  class Mesh;

////////////////////////////////////////////////////////////////////////////////

/// GenerateLine3D component class
/// Create a line in 3D
class Mesh_API GenerateLine3D : public MeshGenerator {

public: // functions

  /// Contructor
  /// @param name of the component
  GenerateLine3D ( const std::string& name );

  /// Virtual destructor
  virtual ~GenerateLine3D();

  /// Get the class name
  static std::string type_name () { return "GenerateLine3D"; }
  
  virtual void execute();

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_GenerateLine3D_hpp
