// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_actions_BuildFaceNormals_hpp
#define cf3_mesh_actions_BuildFaceNormals_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/MeshTransformer.hpp"

#include "mesh/actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {
  
//////////////////////////////////////////////////////////////////////////////

/// This class defines a mesh transformer
/// that returns information about the mesh
/// @author Willem Deconinck
class mesh_actions_API BuildFaceNormals : public MeshTransformer
{

public: // functions
  
  /// constructor
  BuildFaceNormals( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "BuildFaceNormals"; }

  virtual void execute();
    
  enum {FIRST=0};
}; // end BuildFaceNormals


////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_actions_BuildFaceNormals_hpp
