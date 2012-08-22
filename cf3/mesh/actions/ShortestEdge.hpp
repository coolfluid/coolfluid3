// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_actions_ShortestEdge_hpp
#define cf3_mesh_actions_ShortestEdge_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/MeshTransformer.hpp"

#include "mesh/actions/LibActions.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

//////////////////////////////////////////////////////////////////////////////

/// Calculates the length of the shortest distance between two nodes in a mesh
/// @author Bart Janssens
class mesh_actions_API ShortestEdge : public MeshTransformer
{
public: // functions

  /// constructor
  ShortestEdge( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "ShortestEdge"; }

  virtual void execute();

}; // end ShortestEdge


////////////////////////////////////////////////////////////////////////////////

} // actions
} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_actions_ShortestEdge_hpp
