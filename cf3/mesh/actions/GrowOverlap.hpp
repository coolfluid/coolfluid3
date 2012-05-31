// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_actions_GrowOverlap_hpp
#define cf3_mesh_actions_GrowOverlap_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/MeshTransformer.hpp"

#include "mesh/actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

//////////////////////////////////////////////////////////////////////////////

/// @brief Grow the overlap of the mesh with one layer
///
/// Boundary nodes of one rank are communicated to other ranks.
/// Each other rank then communicates all elements that are connected
/// to these boundary nodes.
/// Missing nodes are then also communicated to complete the elements
///
/// @author Willem Deconinck
class mesh_actions_API GrowOverlap : public MeshTransformer
{
public: // functions

  /// constructor
  GrowOverlap( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "GrowOverlap"; }

  virtual void execute();

}; // end GrowOverlap


////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_GrowOverlap_hpp
