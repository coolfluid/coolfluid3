// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_MeshTriangulator_hpp
#define cf3_mesh_MeshTriangulator_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/MeshTransformer.hpp"
#include "mesh/LibMesh.hpp"

namespace cf3 {
namespace mesh {

/// MeshTriangulator component class
/// This class finds quadrilateral zones in a 2D mesh and replaces them with triangles
/// @author Bart Janssens
class Mesh_API MeshTriangulator : public MeshTransformer
{
public: // functions

  /// Contructor
  /// @param name of the component
  MeshTriangulator ( const std::string& name );

  /// Virtual destructor
  virtual ~MeshTriangulator();

  /// Get the class name
  static std::string type_name () { return "MeshTriangulator"; }

  virtual void execute();
};

  ////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_MeshTriangulator_hpp
