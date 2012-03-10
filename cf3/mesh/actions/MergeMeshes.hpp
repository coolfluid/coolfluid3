// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_actions_MergeMeshes_hpp
#define cf3_mesh_actions_MergeMeshes_hpp

////////////////////////////////////////////////////////////////////////////////

#include "math/MatrixTypes.hpp"
#include "common/Action.hpp"
#include "mesh/actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
class Mesh;
namespace actions {
  
//////////////////////////////////////////////////////////////////////////////

/// @brief Merge several meshes into a new mesh
/// @author Willem Deconinck
class mesh_actions_API MergeMeshes : public common::Component
{
public: // functions
  
  /// Gets the Class name
  static std::string type_name() { return "MergeMeshes"; }

  /// Constructor
  MergeMeshes( const std::string& name );

  /// Virtual destructor
  virtual ~MergeMeshes() {};

  /// Merge meshes
  void merge_mesh(const Mesh& mesh, Mesh& merged_mesh);

  void fix_ranks(Mesh& merged_mesh);

}; // end MergeMeshes

////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_MergeMeshes_hpp
